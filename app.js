(() => {
  const KEY = "nova-workspace-v1";
  const blank = () => ({notes:[],tasks:[],projects:[]});
  let data;
  try { data = JSON.parse(localStorage.getItem(KEY)) || blank(); } catch { data = blank(); }
  data.notes ||= []; data.tasks ||= []; data.projects ||= [];
  let currentView = "overview";
  let toastTimer;
  const $ = (s) => document.querySelector(s);
  const esc = (s="") => String(s).replace(/[&<>"']/g, c => ({"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c]));
  const save = () => { try { localStorage.setItem(KEY, JSON.stringify(data)); } catch { notify("Storage is full. Export a backup and remove old items."); } };
  const notify = (msg) => { const el=$("#toast"); el.textContent=msg; el.classList.add("show"); clearTimeout(toastTimer); toastTimer=setTimeout(()=>el.classList.remove("show"),2400); };
  const date = (v) => new Date(v).toLocaleDateString(undefined,{month:"short",day:"numeric",year:"numeric"});
  const title = {overview:"Overview",notes:"Notes",tasks:"Tasks",projects:"Projects"};
  const setView = (v) => { currentView=v; document.querySelectorAll(".nav-item").forEach(b=>b.classList.toggle("active",b.dataset.view===v)); $("#pageTitle").textContent=title[v]||"NOVA"; render(); };
  const empty = (msg) => '<div class="empty">'+msg+'</div>';
  const stat = (label,n) => '<div class="stat"><span>'+label+'</span><strong>'+n+'</strong></div>';
  function noteCard(n){return '<article class="item-card"><div class="item-meta">'+date(n.updated||n.created)+'</div><h3>'+esc(n.title||"Untitled note")+'</h3><p>'+esc(n.body||"No content yet")+'</p><div class="card-actions"><button class="small-btn" data-edit-note="'+n.id+'">Edit</button><button class="small-btn danger" data-delete-note="'+n.id+'">Delete</button></div></article>'}
  function taskCard(t){return '<article class="item-card"><div class="task-row"><input type="checkbox" data-toggle-task="'+t.id+'" '+(t.done?"checked":"")+' aria-label="Mark task complete"><div style="min-width:0;flex:1"><h3 class="'+(t.done?"task-done":"")+'">'+esc(t.title)+'</h3><div class="item-meta">'+(t.due?"Due "+date(t.due):"No due date")+(t.project?" · "+esc(t.project):"")+'</div><div class="card-actions"><button class="small-btn danger" data-delete-task="'+t.id+'">Delete</button></div></div></div></article>'}
  function projectCard(p){let count=data.tasks.filter(t=>t.project===p.name).length;return '<article class="item-card"><span class="pill">PROJECT</span><h3 style="margin-top:12px">'+esc(p.name)+'</h3><p>'+esc(p.description||"A space for your next idea.")+'</p><div class="item-meta">'+count+' task(s) · Created '+date(p.created)+'</div><div class="card-actions"><button class="small-btn danger" data-delete-project="'+p.id+'">Delete</button></div></article>'}
  function render(){
    const q=$("#searchInput").value.trim().toLowerCase();
    if(q && currentView!=="overview"){
      let list=[];
      if(currentView==="notes") list=data.notes.filter(x=>(x.title+" "+x.body).toLowerCase().includes(q)).map(noteCard);
      if(currentView==="tasks") list=data.tasks.filter(x=>(x.title+" "+x.project).toLowerCase().includes(q)).map(taskCard);
      if(currentView==="projects") list=data.projects.filter(x=>(x.name+" "+x.description).toLowerCase().includes(q)).map(projectCard);
      $("#view").innerHTML='<div class="section-head"><h2>Search results</h2><span class="muted">'+list.length+' found</span></div><div class="item-grid">'+(list.join("")||empty("No matching items found."))+'</div>'; return;
    }
    if(currentView==="overview"){
      const recent=[...data.notes].sort((a,b)=>(b.updated||b.created)-(a.updated||a.created)).slice(0,2);
      $("#view").innerHTML='<section class="hero"><span class="eyebrow">WELCOME TO YOUR WORKSPACE</span><h2>Make room for what matters.</h2><p>NOVA is your simple home for ideas, plans and projects. Capture a thought, organise a task, and keep moving — all in one place.</p><button class="primary-btn" data-action="note">＋ Create a note</button></section><div class="stats">'+stat("NOTES",data.notes.length)+stat("OPEN TASKS",data.tasks.filter(t=>!t.done).length)+stat("PROJECTS",data.projects.length)+'</div><div class="section-head"><h2>Quick actions</h2><span class="muted">Start something</span></div><div class="quick-grid"><button class="item-card quick-card" data-action="task"><b>＋ Add a task</b><span>Turn your next step into action.</span></button><button class="item-card quick-card" data-action="project"><b>◫ Create a project</b><span>Give a bigger idea its own space.</span></button></div><div class="section-head"><h2>Recently updated notes</h2><button class="small-btn" data-view="notes">View all</button></div><div class="item-grid">'+(recent.map(noteCard).join("")||empty("Your notes will appear here. Create one to get started."))+'</div>';
    } else if(currentView==="notes"){
      $("#view").innerHTML='<div class="toolbar"><span class="muted">Keep useful thoughts close.</span><button class="primary-btn" data-action="note">＋ New note</button></div><div class="item-grid">'+(data.notes.slice().sort((a,b)=>(b.updated||b.created)-(a.updated||a.created)).map(noteCard).join("")||empty("No notes yet. Create your first note."))+'</div>';
    } else if(currentView==="tasks"){
      $("#view").innerHTML='<div class="toolbar"><span class="muted">Small steps add up.</span><button class="primary-btn" data-action="task">＋ New task</button></div><div class="item-grid">'+(data.tasks.slice().sort((a,b)=>Number(a.done)-Number(b.done)).map(taskCard).join("")||empty("Nothing on your task list yet."))+'</div>';
    } else {
      $("#view").innerHTML='<div class="toolbar"><span class="muted">Keep related work together.</span><button class="primary-btn" data-action="project">＋ New project</button></div><div class="item-grid">'+(data.projects.map(projectCard).join("")||empty("No projects yet. Create one for something you are working on."))+'</div>';
    }
  }
  function form(type,id){
    const existing=type==="note"?data.notes.find(x=>x.id===id):null;
    const task=type==="task"?data.tasks.find(x=>x.id===id):null;
    const project=type==="project"?data.projects.find(x=>x.id===id):null;
    const options=data.projects.map(p=>'<option value="'+esc(p.name)+'" '+((task?.project===p.name)?"selected":"")+'>'+esc(p.name)+'</option>').join("");
    const fields= type==="note" ? '<label>Title<input name="title" maxlength="100" required value="'+esc(existing?.title||"")+'" placeholder="A thought worth keeping"></label><label>Content<textarea name="body" placeholder="Write anything...">'+esc(existing?.body||"")+'</textarea></label>' :
      type==="task" ? '<label>Task<input name="title" maxlength="120" required value="'+esc(task?.title||"")+'" placeholder="What needs to be done?"></label><label>Due date<input name="due" type="date" value="'+esc(task?.due||"")+'"></label><label>Project<select name="project"><option value="">No project</option>'+options+'</select></label>' :
      '<label>Project name<input name="name" maxlength="70" required value="'+esc(project?.name||"")+'" placeholder="My next big idea"></label><label>Description<textarea name="description" placeholder="What is this project about?">'+esc(project?.description||"")+'</textarea></label>';
    $("#view").innerHTML='<div class="panel"><h3>'+(id?"Edit ":"Create ")+type+'</h3><form id="itemForm" class="editor">'+fields+'<div class="editor-actions"><button type="button" class="small-btn" data-cancel>Cancel</button><button class="primary-btn" type="submit">Save '+type+'</button></div></form></div>';
    $("#itemForm").addEventListener("submit",e=>{e.preventDefault();const f=new FormData(e.currentTarget);const now=Date.now();
      if(type==="note"){const obj={id:id||crypto.randomUUID(),title:f.get("title").trim(),body:f.get("body").trim(),created:existing?.created||now,updated:now};if(existing)Object.assign(existing,obj);else data.notes.unshift(obj);}
      if(type==="task"){const obj={id:id||crypto.randomUUID(),title:f.get("title").trim(),due:f.get("due"),project:f.get("project"),done:task?.done||false,created:task?.created||now};if(task)Object.assign(task,obj);else data.tasks.unshift(obj);}
      if(type==="project"){const obj={id:id||crypto.randomUUID(),name:f.get("name").trim(),description:f.get("description").trim(),created:project?.created||now};if(project)Object.assign(project,obj);else data.projects.unshift(obj);}
      save();notify(type[0].toUpperCase()+type.slice(1)+" saved");setView(type==="note"?"notes":type==="task"?"tasks":"projects");
    });
  }
  function exportData(){const blob=new Blob([JSON.stringify({format:"NOVA-backup",version:1,exportedAt:new Date().toISOString(),data},null,2)],{type:"application/json"});const a=document.createElement("a");a.href=URL.createObjectURL(blob);a.download="nova-workspace-backup.json";a.click();URL.revokeObjectURL(a.href);notify("Backup exported");}
  function importData(file){const reader=new FileReader();reader.onload=()=>{try{const obj=JSON.parse(reader.result);const d=obj.data||obj;if(!Array.isArray(d.notes)||!Array.isArray(d.tasks)||!Array.isArray(d.projects))throw Error("Invalid backup");if(!confirm("Replace this browser's NOVA data with the selected backup?"))return;data=d;save();render();notify("Backup imported");}catch{notify("That file is not a valid NOVA backup");}};reader.readAsText(file);}
  document.addEventListener("click",e=>{
    const b=e.target.closest("button");if(!b)return;
    if(b.dataset.view){setView(b.dataset.view);return;}
    if(b.dataset.action){form(b.dataset.action);return;}
    if(b.dataset.cancel){render();return;}
    if(b.dataset.editNote){form("note",b.dataset.editNote);return;}
    if(b.dataset.deleteNote){if(confirm("Delete this note?")){data.notes=data.notes.filter(x=>x.id!==b.dataset.deleteNote);save();render();notify("Note deleted");}return;}
    if(b.dataset.deleteTask){if(confirm("Delete this task?")){data.tasks=data.tasks.filter(x=>x.id!==b.dataset.deleteTask);save();render();notify("Task deleted");}return;}
    if(b.dataset.deleteProject){if(confirm("Delete this project? Tasks will remain, but become unassigned.")){const p=data.projects.find(x=>x.id===b.dataset.deleteProject);data.tasks.forEach(t=>{if(t.project===p?.name)t.project="";});data.projects=data.projects.filter(x=>x.id!==b.dataset.deleteProject);save();render();notify("Project deleted");}return;}
  });
  document.addEventListener("change",e=>{if(e.target.matches("[data-toggle-task]")){const t=data.tasks.find(x=>x.id===e.target.dataset.toggleTask);if(t){t.done=e.target.checked;save();render();}}});
  document.querySelectorAll(".nav-item").forEach(b=>b.addEventListener("click",()=>setView(b.dataset.view)));
  $("#newBtn").addEventListener("click",()=>form(currentView==="notes"?"note":currentView==="tasks"?"task":currentView==="projects"?"project":"note"));
  $("#searchInput").addEventListener("input",render);
  $("#exportBtn").addEventListener("click",exportData);
  $("#importBtn").addEventListener("click",()=>$("#importFile").click());
  $("#importFile").addEventListener("change",e=>{if(e.target.files[0])importData(e.target.files[0]);e.target.value="";});
  if("serviceWorker" in navigator && location.protocol!=="file:")window.addEventListener("load",()=>navigator.serviceWorker.register("./service-worker.js").catch(()=>{}));
  setView("overview");
})();