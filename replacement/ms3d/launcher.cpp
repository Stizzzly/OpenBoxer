#include <cstring>
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>
#include <filesystem>
#include <string>
#include <stdexcept>

static_assert(sizeof(void*) == 4, "injector must be x86");
namespace fs = std::filesystem;
struct Handle {
    HANDLE value = nullptr;
    ~Handle() { if(value && value != INVALID_HANDLE_VALUE) CloseHandle(value); }
};
static void check(bool ok, const char* what) {
    if(!ok) throw std::runtime_error(std::string(what)+" (Win32="+std::to_string(GetLastError())+")");
}
static uintptr_t moduleBase(DWORD pid, const char* name) {
    Handle snap{CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid)};
    check(snap.value != INVALID_HANDLE_VALUE, "module snapshot");
    MODULEENTRY32 entry{}; entry.dwSize=sizeof(entry);
    if(Module32First(snap.value,&entry)) do {
        if(_stricmp(entry.szModule,name)==0) return reinterpret_cast<uintptr_t>(entry.modBaseAddr);
    } while(Module32Next(snap.value,&entry));
    throw std::runtime_error("required remote module absent");
}
static DWORD remoteCall(HANDLE process, uintptr_t entry, void* argument,DWORD timeoutMs=60000) {
    Handle thread{CreateRemoteThread(process,nullptr,0,reinterpret_cast<LPTHREAD_START_ROUTINE>(entry),argument,0,nullptr)};
    check(thread.value!=nullptr,"remote thread");
    check(WaitForSingleObject(thread.value,timeoutMs)==WAIT_OBJECT_0,"remote thread timeout");
    DWORD result=0; check(GetExitCodeThread(thread.value,&result)!=0,"remote thread result");
    return result;
}
static void inject(HANDLE process,DWORD pid,const fs::path& dll,bool fixture,bool worldFixture=false,bool textureFixture=false,bool uploadFixture=false,bool lightingFixture=false,bool selectionFixture=false,bool drawFixture=false,bool characterFixture=false,bool characterReplay=false,bool animationFixture=false,bool animationReplay=false,bool clipFixture=false,bool clipReplay=false,bool framesFixture=false,bool framesReplay=false,bool actionFixture=false,bool actionReplay=false,bool strikeFixture=false,bool strikeReplay=false) {
    const auto dllText=dll.string();
    void* remote=VirtualAllocEx(process,nullptr,dllText.size()+1,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    check(remote!=nullptr,"remote allocation");
    SIZE_T written=0;
    check(WriteProcessMemory(process,remote,dllText.c_str(),dllText.size()+1,&written)!=0 && written==dllText.size()+1,"DLL path write");
    HMODULE kernel=GetModuleHandleA("kernel32.dll"); FARPROC load=GetProcAddress(kernel,"LoadLibraryA"); HMODULE owner=nullptr;
    check(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<const char*>(load),&owner)!=0,"resolve LoadLibrary owner");
    char ownerPath[MAX_PATH]{}; check(GetModuleFileNameA(owner,ownerPath,MAX_PATH)!=0,"LoadLibrary owner path");
    const auto loadOffset=reinterpret_cast<uintptr_t>(load)-reinterpret_cast<uintptr_t>(owner);
    const DWORD loaded=remoteCall(process,moduleBase(pid,fs::path(ownerPath).filename().string().c_str())+loadOffset,remote);
    check(loaded!=0,"remote LoadLibrary"); check(VirtualFreeEx(process,remote,0,MEM_RELEASE)!=0,"remote path free");
    HMODULE local=LoadLibraryExA(dllText.c_str(),nullptr,DONT_RESOLVE_DLL_REFERENCES); check(local!=nullptr,"map replacement export");
    const auto invoke=[&](const char* name,void* argument) { FARPROC exportAddress=GetProcAddress(local,name); check(exportAddress!=nullptr,"replacement export missing"); return remoteCall(process,loaded+reinterpret_cast<uintptr_t>(exportAddress)-reinterpret_cast<uintptr_t>(local),argument,characterReplay?300000:60000); };
    const DWORD bootstrap=invoke("ms3d_bootstrap",strikeReplay?reinterpret_cast<void*>(21):strikeFixture?reinterpret_cast<void*>(20):actionReplay?reinterpret_cast<void*>(19):actionFixture?reinterpret_cast<void*>(18):framesReplay?reinterpret_cast<void*>(17):framesFixture?reinterpret_cast<void*>(16):clipReplay?reinterpret_cast<void*>(15):clipFixture?reinterpret_cast<void*>(14):animationReplay?reinterpret_cast<void*>(13):animationFixture?reinterpret_cast<void*>(12):characterReplay?reinterpret_cast<void*>(11):characterFixture?reinterpret_cast<void*>(10):drawFixture?reinterpret_cast<void*>(9):selectionFixture?reinterpret_cast<void*>(8):lightingFixture?reinterpret_cast<void*>(7):uploadFixture?reinterpret_cast<void*>(6):textureFixture?reinterpret_cast<void*>(5):worldFixture?reinterpret_cast<void*>(4):fixture?reinterpret_cast<void*>(3):nullptr);
    if(bootstrap!=0) { FreeLibrary(local); throw std::runtime_error("replacement initialization rejected: "+std::to_string(bootstrap)); }
    if(fixture) { const DWORD result=invoke(strikeReplay?"strike_replay_worker":strikeFixture?"strike_fixture_worker":actionReplay?"action_replay_worker":actionFixture?"action_fixture_worker":framesReplay?"frames_replay_worker":framesFixture?"frames_fixture_worker":clipReplay?"clip_replay_worker":clipFixture?"clip_fixture_worker":animationReplay?"animation_replay_worker":animationFixture?"animation_fixture_worker":characterReplay?"character_replay_worker":characterFixture?"character_fixture_worker":drawFixture?"draw_fixture_worker":selectionFixture?"selection_fixture_worker":lightingFixture?"lighting_fixture_worker":uploadFixture?"upload_fixture_worker":textureFixture?"texture_fixture_worker":worldFixture?"world0001_fixture_worker":"ms3d_fixture_worker",nullptr); std::printf("%s fixture worker result=%lu, reports C:/Users/ADMIN/Boxer-lab/ms3d/\n",(strikeFixture || strikeReplay)?"GAME-0001":(actionFixture || actionReplay)?"ANIM-0004":(framesFixture || framesReplay)?"ANIM-0003":(clipFixture || clipReplay)?"ANIM-0002":(animationFixture || animationReplay)?"ANIM-0001":(characterFixture || characterReplay)?"RENDER-0006":drawFixture?"RENDER-0005":selectionFixture?"RENDER-0004":lightingFixture?"RENDER-0003":uploadFixture?"RENDER-0002":textureFixture?"RENDER-0001":worldFixture?"WORLD-0001":"IO-0003",result); check(result==0,"fixture worker differences or failure"); }
    FreeLibrary(local);
}
// The supplied entry RVA is ABI metadata. Hardware breakpoints alter debugger
// state only; no original instruction bytes are read or modified.
static void parkAtInitializedCheckpoint(PROCESS_INFORMATION& process) {
    constexpr uintptr_t checkpointRva=0x4cd90;
    const ULONGLONG deadline=GetTickCount64()+30000;
    uintptr_t checkpoint=0;
    bool ready=false;
    while(!ready && GetTickCount64()<deadline) {
        DEBUG_EVENT event{};
        if(!WaitForDebugEvent(&event,1000)) { if(GetLastError()==ERROR_SEM_TIMEOUT) continue; check(false,"debug startup event"); }
        DWORD status=DBG_CONTINUE;
        if(event.dwDebugEventCode==CREATE_PROCESS_DEBUG_EVENT) {
            checkpoint=reinterpret_cast<uintptr_t>(event.u.CreateProcessInfo.lpBaseOfImage)+checkpointRva;
            CONTEXT context{}; context.ContextFlags=CONTEXT_DEBUG_REGISTERS;
            check(GetThreadContext(process.hThread,&context)!=0,"read startup debug registers");
            context.Dr0=checkpoint; context.Dr6=0; context.Dr7=(context.Dr7&~0xf0003u)|1u;
            check(SetThreadContext(process.hThread,&context)!=0,"arm initialized checkpoint");
            if(event.u.CreateProcessInfo.hFile) CloseHandle(event.u.CreateProcessInfo.hFile);
        } else if(event.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT) {
            if(event.u.LoadDll.hFile) CloseHandle(event.u.LoadDll.hFile);
        } else if(event.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT) {
            if(event.u.CreateThread.hThread) CloseHandle(event.u.CreateThread.hThread);
        } else if(event.dwDebugEventCode==EXCEPTION_DEBUG_EVENT) {
            const auto& exception=event.u.Exception.ExceptionRecord;
            if(exception.ExceptionCode==EXCEPTION_SINGLE_STEP && event.dwThreadId==process.dwThreadId && reinterpret_cast<uintptr_t>(exception.ExceptionAddress)==checkpoint) {
                check(SuspendThread(process.hThread)!=static_cast<DWORD>(-1),"park initialized main thread");
                CONTEXT context{}; context.ContextFlags=CONTEXT_DEBUG_REGISTERS;
                check(GetThreadContext(process.hThread,&context)!=0,"read checkpoint debug registers");
                context.Dr0=0; context.Dr6=0; context.Dr7&=~0xf0003u;
                check(SetThreadContext(process.hThread,&context)!=0,"clear checkpoint breakpoint");
                ready=true;
            } else if(exception.ExceptionCode!=EXCEPTION_BREAKPOINT) status=DBG_EXCEPTION_NOT_HANDLED;
        } else if(event.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT) {
            ContinueDebugEvent(event.dwProcessId,event.dwThreadId,status);
            throw std::runtime_error("test copy exited before CRT-ready checkpoint");
        }
        check(ContinueDebugEvent(event.dwProcessId,event.dwThreadId,status)!=0,"continue startup debug event");
    }
    check(ready,"CRT-ready checkpoint timeout");
    check(DebugSetProcessKillOnExit(FALSE)!=0,"retain own process during debugger detach");
    // Windows can still have a queued loader/thread event when the checkpoint
    // event has just been continued. Complete each pending event before retrying
    // detach; the explicit primary-thread suspension remains in force.
    DWORD detachError=0;
    for(unsigned attempt=0;attempt<30;attempt++) {
        if(DebugActiveProcessStop(process.dwProcessId)) return;
        detachError=GetLastError();
        DEBUG_EVENT pending{};
        if(WaitForDebugEvent(&pending,100)) {
            DWORD continuation=DBG_CONTINUE;
            if(pending.dwDebugEventCode==LOAD_DLL_DEBUG_EVENT && pending.u.LoadDll.hFile) CloseHandle(pending.u.LoadDll.hFile);
            if(pending.dwDebugEventCode==CREATE_THREAD_DEBUG_EVENT && pending.u.CreateThread.hThread) CloseHandle(pending.u.CreateThread.hThread);
            if(pending.dwDebugEventCode==EXCEPTION_DEBUG_EVENT && pending.u.Exception.ExceptionRecord.ExceptionCode!=EXCEPTION_BREAKPOINT) continuation=DBG_EXCEPTION_NOT_HANDLED;
            check(ContinueDebugEvent(pending.dwProcessId,pending.dwThreadId,continuation)!=0,"complete pending startup event");
            if(pending.dwDebugEventCode==EXIT_PROCESS_DEBUG_EVENT) throw std::runtime_error("test copy exited during startup detach");
        }
    }
    SetLastError(detachError); check(false,"detach startup debugger");
}
int main(int argc, char** argv) {
    PROCESS_INFORMATION process{};
    Handle processHandle, mainThread;
    try {
        if(argc==5 && std::strcmp(argv[1],"--attach")==0 && std::strcmp(argv[4],"--fixture")==0) {
            const DWORD pid=static_cast<DWORD>(std::stoul(argv[2]));
            Handle attached{OpenProcess(PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_QUERY_INFORMATION,FALSE,pid)};
            check(attached.value!=nullptr,"open explicit target PID");
            char image[MAX_PATH]{}; DWORD size=MAX_PATH;
            check(QueryFullProcessImageNameA(attached.value,0,image,&size)!=0 && _stricmp(image,"C:\\Users\\ADMIN\\Boxer-lab\\ms3d\\program.exe")==0,"attach requires exact test-copy process");
            inject(attached.value,pid,fs::canonical(argv[3]),true); return 0;
        }
        if(argc!=3) throw std::runtime_error("usage: ms3d_launcher <absolute replacement DLL path> <original|pass-through|shadow|replace>");
        const bool worldFixtures=std::strcmp(argv[1],"--world-fixture")==0;
        const bool textureFixtures=std::strcmp(argv[1],"--texture-fixture")==0;
        const bool uploadFixtures=std::strcmp(argv[1],"--upload-fixture")==0;
        const bool lightingFixtures=std::strcmp(argv[1],"--lighting-fixture")==0;
        const bool selectionFixtures=std::strcmp(argv[1],"--selection-fixture")==0;
        const bool strikeFixtures=std::strcmp(argv[1],"--strike-fixture")==0;
        const bool strikeReplays=std::strcmp(argv[1],"--strike-replay")==0;
        const bool actionFixtures=std::strcmp(argv[1],"--action-fixture")==0;
        const bool actionReplays=std::strcmp(argv[1],"--action-replay")==0;
        const bool framesFixtures=std::strcmp(argv[1],"--frames-fixture")==0;
        const bool framesReplays=std::strcmp(argv[1],"--frames-replay")==0;
        const bool clipFixtures=std::strcmp(argv[1],"--clip-fixture")==0;
        const bool clipReplays=std::strcmp(argv[1],"--clip-replay")==0;
        const bool animationFixtures=std::strcmp(argv[1],"--animation-fixture")==0;
        const bool animationReplays=std::strcmp(argv[1],"--animation-replay")==0;
        const bool characterReplays=std::strcmp(argv[1],"--character-replay")==0;
        const bool characterFixtures=std::strcmp(argv[1],"--character-fixture")==0;
        const bool drawFixtures=std::strcmp(argv[1],"--draw-fixture")==0;
        const bool runFixtures=std::strcmp(argv[1],"--fixture")==0 || worldFixtures || textureFixtures || uploadFixtures || lightingFixtures || selectionFixtures || drawFixtures || characterFixtures || characterReplays || animationFixtures || animationReplays || clipFixtures || clipReplays || framesFixtures || framesReplays || actionFixtures || actionReplays || strikeFixtures || strikeReplays;
        const std::string mode=runFixtures?"replace":argv[2];
        if(mode!="original" && mode!="pass-through" && mode!="shadow" && mode!="replace") throw std::runtime_error("invalid mode");
        const fs::path lab="C:/Users/ADMIN/Boxer-lab/ms3d";
        const auto marker=lab/"MS3D_TEST_COPY.marker";
        check(fs::exists(marker),"test-copy marker missing");
        const auto dll=fs::canonical(runFixtures?argv[2]:argv[1]);
        check(fs::is_regular_file(dll),"replacement DLL missing");
        check(SetEnvironmentVariableA("OPENBOXER_MS3D_MODE",mode.c_str())!=0,"mode environment");
        const auto exe=lab/"program.exe";
        std::string command='"'+exe.string()+'"';
        STARTUPINFOA startup{}; startup.cb=sizeof(startup);
        check(CreateProcessA(exe.string().c_str(),command.data(),nullptr,nullptr,FALSE,DEBUG_ONLY_THIS_PROCESS,nullptr,lab.string().c_str(),&startup,&process)!=0,"launch explicit test copy");
        processHandle.value=process.hProcess; mainThread.value=process.hThread;
        std::printf("Created own explicit test copy PID %lu; waiting for CRT-ready checkpoint.\n",process.dwProcessId); std::fflush(stdout);
        parkAtInitializedCheckpoint(process);
        char guardDelay[32]{};
        if(GetEnvironmentVariableA("OPENBOXER_WORLD_GUARD_DELAY_MS",guardDelay,sizeof(guardDelay))) {
            const unsigned long requested=std::stoul(guardDelay);
            Sleep((std::min)(requested,5000ul));
        }
        if(mode!="original") inject(process.hProcess,process.dwProcessId,dll,runFixtures,worldFixtures,textureFixtures,uploadFixtures,lightingFixtures,selectionFixtures,drawFixtures,characterFixtures,characterReplays,animationFixtures,animationReplays,clipFixtures,clipReplays,framesFixtures,framesReplays,actionFixtures,actionReplays,strikeFixtures,strikeReplays);
        if(runFixtures) { check(TerminateProcess(process.hProcess,0)!=0,"terminate own completed fixture process"); return 0; }
        check(ResumeThread(process.hThread)!=static_cast<DWORD>(-1),"resume test copy");
        std::printf("Started explicit test copy PID %lu in %s mode.\n",process.dwProcessId,mode.c_str());
        return 0;
    } catch(const std::exception& error) {
        if(process.hProcess) TerminateProcess(process.hProcess,1);
        std::fprintf(stderr,"%s\n",error.what());
        return 1;
    }
}








