#include "xboxlab/manifest_probe.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace xboxlab {
namespace {
constexpr std::size_t kMaxManifestBytes = 64U * 1024U;
constexpr std::size_t kMaxDepth = 16;
constexpr std::size_t kMaxTokens = 2048;
constexpr std::size_t kMaxAttributes = 32;
constexpr std::size_t kMaxAttributeBytes = 4096;

bool name_start(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return std::isalpha(u) != 0 || c == '_' || c == ':';
}
bool name_char(char c) {
    const unsigned char u = static_cast<unsigned char>(c);
    return name_start(c) || std::isdigit(u) != 0 || c == '-' || c == '.';
}
void append_utf8(std::string& out, std::uint32_t cp) {
    if (cp <= 0x7F) out.push_back(static_cast<char>(cp));
    else if (cp <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0U | (cp >> 6U)));
        out.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else if (cp <= 0xFFFF && (cp < 0xD800 || cp > 0xDFFF)) {
        out.push_back(static_cast<char>(0xE0U | (cp >> 12U)));
        out.push_back(static_cast<char>(0x80U | ((cp >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    } else if (cp <= 0x10FFFF) {
        out.push_back(static_cast<char>(0xF0U | (cp >> 18U)));
        out.push_back(static_cast<char>(0x80U | ((cp >> 12U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | ((cp >> 6U) & 0x3FU)));
        out.push_back(static_cast<char>(0x80U | (cp & 0x3FU)));
    }
}
bool decode_attribute(std::string_view raw, std::string& decoded) {
    decoded.clear();
    for (std::size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] != '&') {
            if (raw[i] == '\0') return false;
            decoded.push_back(raw[i]);
            continue;
        }
        const std::size_t semi = raw.find(';', i + 1);
        if (semi == std::string_view::npos || semi - i > 12) return false;
        const auto entity = raw.substr(i + 1, semi - i - 1);
        if (entity == "amp") decoded.push_back('&');
        else if (entity == "lt") decoded.push_back('<');
        else if (entity == "gt") decoded.push_back('>');
        else if (entity == "quot") decoded.push_back('"');
        else if (entity == "apos") decoded.push_back('\'');
        else if (!entity.empty() && entity[0] == '#') {
            std::uint32_t value = 0;
            std::size_t pos = 1;
            unsigned int base = 10;
            if (pos < entity.size() && (entity[pos] == 'x' || entity[pos] == 'X')) {
                base = 16; ++pos;
            }
            if (pos == entity.size()) return false;
            for (; pos < entity.size(); ++pos) {
                const char c = entity[pos];
                unsigned int digit = 0;
                if (c >= '0' && c <= '9') digit = static_cast<unsigned int>(c - '0');
                else if (base == 16 && c >= 'a' && c <= 'f') digit = 10U + static_cast<unsigned int>(c - 'a');
                else if (base == 16 && c >= 'A' && c <= 'F') digit = 10U + static_cast<unsigned int>(c - 'A');
                else return false;
                if (digit >= base || value > (0x10FFFFU - digit) / base) return false;
                value = value * base + digit;
            }
            if (value == 0 || value > 0x10FFFFU || (value >= 0xD800U && value <= 0xDFFFU) ||\n                (value < 0x20U && value != 0x09U && value != 0x0AU && value != 0x0DU)) return false;
            append_utf8(decoded, value);
        } else return false; // No custom or external entities.
        i = semi;
        if (decoded.size() > kMaxAttributeBytes) return false;
    }
    return decoded.size() <= kMaxAttributeBytes;
}
void skip_space(std::string_view s, std::size_t& p) {
    while (p < s.size() && std::isspace(static_cast<unsigned char>(s[p])) != 0) ++p;
}
bool read_name(std::string_view s, std::size_t& p, std::string& name) {
    if (p >= s.size() || !name_start(s[p])) return false;
    const std::size_t begin = p++;
    while (p < s.size() && name_char(s[p])) ++p;
    name.assign(s.substr(begin, p - begin));
    return true;
}
} // namespace

GameManifest inspect_game_manifest(std::string_view xml) {
    GameManifest result;
    auto fail = [&](const char* message) {
        result.valid = false;
        result.error = message;
        result.executables.clear();
        return result;
    };
    if (xml.empty() || xml.size() > kMaxManifestBytes) return fail("Manifest is empty or exceeds the 64 KiB limit.");
    if (xml.find('\0') != std::string_view::npos) return fail("Manifest contains a NUL byte.");

    std::vector<std::string> stack;
    std::size_t pos = 0;
    std::size_t tokens = 0;
    bool root_seen = false;
    bool root_closed = false;
    bool identity_seen = false;
    bool executable_list_seen = false;
    bool shell_visuals_seen = false;
    bool msa_app_id_seen = false;
    bool title_id_seen = false;

    while (pos < xml.size()) {
        const std::size_t open = xml.find('<', pos);
        if (open == std::string_view::npos) {
            for (; pos < xml.size(); ++pos) {
                if (!std::isspace(static_cast<unsigned char>(xml[pos]))) return fail("Unexpected text after the root element.");
            }
            break;
        }
        if (open > pos) {
            const auto raw_text = xml.substr(pos, open - pos);
            if (stack.empty()) {
                for (char ch : raw_text) {
                    if (std::isspace(static_cast<unsigned char>(ch)) == 0)
                        return fail("Text outside the root element is not allowed.");
                }
            } else {
                std::string decoded_text;
                if (!decode_attribute(raw_text, decoded_text))
                    return fail("Unsupported or invalid XML entity in element text.");
                auto first = decoded_text.find_first_not_of(" \\t\\r\\n");
                auto last = decoded_text.find_last_not_of(" \\t\\r\\n");
                const std::string value = first == std::string::npos
                    ? std::string{} : decoded_text.substr(first, last - first + 1);
                if (stack.back() == "MSAAppId") {
                    if (msa_app_id_seen) return fail("Duplicate MSAAppId element.");
                    msa_app_id_seen = true;
                    result.msa_app_id = value;
                } else if (stack.back() == "TitleId") {
                    if (title_id_seen) return fail("Duplicate TitleId element.");
                    title_id_seen = true;
                    result.title_id = value;
                }
            }
            pos = open;
        }
        pos = open + 1;
        if (++tokens > kMaxTokens) return fail("Manifest exceeds the XML token limit.");
        if (pos >= xml.size()) return fail("Truncated XML tag.");

        if (xml.substr(pos, 3) == "!--") {
            const auto end = xml.find("-->", pos + 3);
            if (end == std::string_view::npos) return fail("Unterminated XML comment.");
            pos = end + 3;
            continue;
        }
        if (xml[pos] == '?') {
            const auto end = xml.find("?>", pos + 1);
            if (end == std::string_view::npos) return fail("Unterminated XML processing instruction.");
            pos = end + 2;
            continue;
        }
        if (xml[pos] == '!') return fail("DOCTYPE, CDATA, and declarations are not supported.");

        if (xml[pos] == '/') {
            ++pos;
            std::string closing;
            if (!read_name(xml, pos, closing)) return fail("Invalid closing tag.");
            skip_space(xml, pos);
            if (pos >= xml.size() || xml[pos] != '>') return fail("Malformed closing tag.");
            ++pos;
            if (stack.empty() || stack.back() != closing) return fail("Mismatched XML closing tag.");
            stack.pop_back();
            if (stack.empty()) root_closed = true;
            continue;
        }

        std::string tag;
        if (!read_name(xml, pos, tag)) return fail("Invalid XML element name.");
        const std::string parent = stack.empty() ? std::string{} : stack.back();
        if (stack.empty()) {
            if (root_seen || root_closed || tag != "Game") return fail("Expected exactly one Game root element.");
            root_seen = true;
        }
        std::vector<std::pair<std::string, std::string>> attrs;
        bool self_closing = false;
        for (;;) {
            skip_space(xml, pos);
            if (pos >= xml.size()) return fail("Truncated start tag.");
            if (xml[pos] == '>') { ++pos; break; }
            if (xml[pos] == '/') {
                ++pos;
                if (pos >= xml.size() || xml[pos] != '>') return fail("Malformed self-closing tag.");
                ++pos; self_closing = true; break;
            }
            if (attrs.size() >= kMaxAttributes) return fail("Element has too many attributes.");
            std::string key;
            if (!read_name(xml, pos, key)) return fail("Invalid XML attribute name.");
            if (std::any_of(attrs.begin(), attrs.end(), [&](const auto& a) { return a.first == key; }))
                return fail("Duplicate XML attribute.");
            skip_space(xml, pos);
            if (pos >= xml.size() || xml[pos++] != '=') return fail("XML attribute is missing '='.");
            skip_space(xml, pos);
            if (pos >= xml.size() || (xml[pos] != '"' && xml[pos] != '\'')) return fail("XML attribute must be quoted.");
            const char quote = xml[pos++];
            const std::size_t begin = pos;
            const std::size_t end = xml.find(quote, begin);
            if (end == std::string_view::npos || end - begin > kMaxAttributeBytes) return fail("Invalid or oversized XML attribute.");
            std::string value;
            if (!decode_attribute(xml.substr(begin, end - begin), value)) return fail("Unsupported or invalid XML entity.");
            attrs.emplace_back(std::move(key), std::move(value));
            pos = end + 1;
        }
        auto attr = [&](std::string_view key) -> std::string {
            for (const auto& a : attrs) if (a.first == key) return a.second;
            return {};
        };

        if (tag == "Game" && parent.empty()) {
            result.config_version = attr("configVersion");
            if (result.config_version != "0" && result.config_version != "1")
                return fail("Game configVersion must be 0 or 1 in this supported manifest subset.");
        } else if (tag == "Identity" && parent == "Game") {
            if (identity_seen) return fail("Duplicate Identity element.");
            identity_seen = true;
            result.identity_name = attr("Name");
            result.publisher = attr("Publisher");
            result.version = attr("Version");
            if (result.identity_name.empty() || result.publisher.empty())
                return fail("Identity requires non-empty Name and Publisher attributes.");
        } else if (tag == "ExecutableList" && parent == "Game") {
            if (executable_list_seen) return fail("Duplicate ExecutableList element.");
            executable_list_seen = true;
        } else if (tag == "Executable" && parent == "ExecutableList") {
            ManifestExecutable executable;
            executable.name = attr("Name");
            executable.id = attr("Id");
            executable.target_device_family = attr("TargetDeviceFamily");
            const std::string dev = attr("IsDevOnly");
            if (executable.name.empty()) return fail("Executable requires a non-empty Name attribute.");
            if (!dev.empty() && dev != "true" && dev != "false") return fail("IsDevOnly must be true or false.");
            executable.development_only = dev == "true";
            result.executables.push_back(std::move(executable));
            if (result.executables.size() > 128) return fail("Manifest has too many executable entries.");
        } else if (tag == "ShellVisuals" && parent == "Game") {
            if (shell_visuals_seen) return fail("Duplicate ShellVisuals element.");
            shell_visuals_seen = true;
            result.display_name = attr("DefaultDisplayName");
            result.publisher_display_name = attr("PublisherDisplayName");
        }

        if (!self_closing) {
            if (stack.size() >= kMaxDepth) return fail("Manifest XML nesting exceeds the depth limit.");
            stack.push_back(std::move(tag));
        } else if (stack.empty()) {
            root_closed = true;
        }
    }

    if (!root_seen || !root_closed || !stack.empty()) return fail("Manifest XML is incomplete.");
    if (!identity_seen) return fail("Manifest is missing its required Identity element.");
    if (!executable_list_seen || result.executables.empty())
        return fail("Manifest requires an ExecutableList with at least one Executable.");
    result.valid = true;
    result.error.clear();
    return result;
}

} // namespace xboxlab
