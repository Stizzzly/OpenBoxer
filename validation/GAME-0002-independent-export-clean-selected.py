from pathlib import Path
v=Path(r'C:\Users\ADMIN\CLionProjects\OpenBoxer\validation')
source=(v/'GAME-0002-independent-export-selected.py').read_text()
start=source.index('choices='); end=source.index('def sha',start)
source=source[:start]+"choices={'clean-stage4':[7,8,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,31,33,34,36,38,39,40]}\ndirs={'clean-stage4':'damage-original-clean-stage4'}\n"+source[end:]
source=source.replace('GAME-0002-approved-source-replay','GAME-0002-approved-clean-source-replay').replace('GAME-0002-independent-approved-source-manifest.json','GAME-0002-independent-approved-clean-source-manifest.json')
exec(compile(source,str(v/'GAME-0002-independent-export-selected.py'),'exec'))
