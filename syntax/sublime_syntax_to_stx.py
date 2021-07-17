import json
import struct
import yaml



SUBLIME_SYNTAX_FILE_PATH="json.sublime-syntax"
with open("mapping.json","r") as f:
	SUBLIME_SCOPE_MAP=json.loads(f.read())



def _parse_scope(sc):
	o=[]
	for k in sc.split(" "):
		k=k.lower()
		for e,v in SUBLIME_SCOPE_MAP.items():
			if (k.startswith(e.lower())):
				if (v is not None):
					o.append(v)
				break
	return o



def _parse_context(ctx,nm_m,p_dt,dt_l):
	dt=dt_l[ctx]
	o=[]
	i=True
	inc_p=True
	for k in dt:
		if ("meta_scope" in k):
			o.append({"type":"global_name","name":_parse_scope(k["meta_scope"])})
			i=False
		elif ("meta_include_prototype" in k):
			inc_p=False
		elif ("clear_scopes" in k):
			if (k["clear_scopes"]==True or k["clear_scopes"]>0):
				o.append({"type":"rewind","value":(0 if k["clear_scopes"]==True else k["clear_scopes"])})
			i=False
		elif ("match" in k):
			o.append({"type":"regex","regex":k["match"],"groups":({e:_parse_scope(ev) for e,ev in k["captures"].items()} if "captures" in k else []),"name":(_parse_scope(k["scope"]) if "scope" in k else None),"push":((nm_m.index(k["push"]) if type(k["push"])==str else len(nm_m)) if "push" in k else -1),"pop":(k["pop"] if "pop" in k else False)})
			if ("push" in k and type(k["push"])!=str):
				nm=f"__anonymous{len(nm_m)}__"
				dt_l[nm]=k["push"]
				nm_m.append(nm)
			i=False
		elif ("include" in k):
			o.append({"type":"link","index":k["include"]})
		else:
			print(k)
	if (p_dt is not None and inc_p is True):
		return p_dt+o
	return o



with open(SUBLIME_SYNTAX_FILE_PATH,"rb") as f:
	dt=yaml.safe_load(f.read())
if ("extends" in dt and len(dt["extends"])>0):
	raise RuntimeError("File Contains Other Syntaxes")
o={"name":dt["name"],"extensions":dt["file_extensions"],"data":[]}
nm_m=["main"]
for k in dt["contexts"]:
	if (k!="main" and k!="prototype"):
		nm_m.append(k)
p_dt=([] if "prototype" not in dt["contexts"] else _parse_context("prototype",nm_m,None,dt["contexts"]))
for k in nm_m:
	o["data"].append(_parse_context(k,nm_m,p_dt,dt["contexts"]))
with open(f"../data/{SUBLIME_SYNTAX_FILE_PATH.split('/')[-1].split('.')[0].lower()}.stx","w") as f:
	f.write(json.dumps(o,indent="\t"))
