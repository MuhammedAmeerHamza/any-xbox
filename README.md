# NOVA Workspace

**A lightweight, private-by-default workspace for notes, tasks and projects.**

NOVA brings a few everyday tools into one calm, installable workspace. The first release is deliberately small, fast and beginner-friendly.

## What you can do

- Write and search notes.
- Track tasks and mark them complete.
- Organise work into projects.
- See a simple overview of your activity.
- Export your workspace as a JSON backup and import it later.
- Install NOVA as an app in supported browsers.
- Keep working offline after the app has loaded once.

## Privacy and storage

This first version stores your information in the browser on the device you use (using local storage). There is no NOVA account, server, analytics, or cloud sync in this release. Your data does not automatically move between devices or browsers. **Export a backup regularly** and keep it somewhere safe.

## Run it

No build tools or programming setup are required.

1. Download or clone this repository.
2. Open `index.html` in a modern browser to try the basic app.
3. For install and offline features, serve the folder from localhost or a static HTTPS host. For example, with Python installed, run `python -m http.server 8000` in the project folder and open `http://localhost:8000`.

You can also publish the repository with GitHub Pages: open **Settings → Pages**, choose **Deploy from a branch**, select **main** and **/(root)**, then save. GitHub will provide the public site URL after deployment.

## Install on a computer or phone

On a supported browser, open the hosted HTTPS site and use the browser's **Install app** or **Add to Home Screen** option. Availability and wording vary by browser and operating system. This is a Progressive Web App (PWA), not a native Windows/macOS/Linux or App Store package.

## Roadmap

- [x] Local-first notes, tasks and projects
- [x] Search and JSON backup/restore
- [x] Installable PWA foundation and offline cache
- [ ] Optional encrypted sync between devices
- [ ] User-controlled integrations and automation
- [ ] Community extension API
- [ ] Native desktop packaging

Roadmap items are plans, not features that are already available.

## Contributing

Issues, bug reports, accessibility improvements, documentation and pull requests are welcome. Read [CONTRIBUTING.md](CONTRIBUTING.md) before submitting changes.

## License

NOVA Workspace is released under the [MIT License](LICENSE).
