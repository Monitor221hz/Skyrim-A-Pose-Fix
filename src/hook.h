#pragma once
#include <cstring>
#include <unordered_set>
#include "RE/H/hkbBehaviorReferenceGenerator.h"
namespace APoseFix 
{
    // Verifies that a_site holds a 5-byte relative CALL (E8 rel32) whose target is the
    // function identified by a_callee. Hooks are only written when this passes, so a future
    // game update that moves code makes the plugin skip the hook (and log it) instead of
    // patching random bytes and crashing.
    inline bool VerifyCallSite(std::uintptr_t a_site, REL::RelocationID a_callee, std::string_view a_name)
    {
        const auto* bytes = reinterpret_cast<const std::uint8_t*>(a_site);
        if (!a_site || bytes[0] != 0xE8) {
            SKSE::log::critical("[{}] patch site {:X} is not a CALL (byte {:02X}) - hook skipped", a_name, a_site, a_site ? bytes[0] : 0);
            return false;
        }
        std::int32_t rel = 0;
        std::memcpy(&rel, bytes + 1, sizeof(rel));
        const auto target   = a_site + 5 + static_cast<std::intptr_t>(rel);
        const auto expected = a_callee.address();
        if (target != expected) {
            SKSE::log::critical("[{}] CALL at {:X} goes to {:X}, expected {:X} - hook skipped", a_name, a_site, target, expected);
            return false;
        }
        SKSE::log::info("[{}] call site verified at {:X}", a_name, a_site);
        return true;
    }
    class LoadBehaviorGraphHook
    {
        public:
        static void Install()
        {
            // Down	p	sub_140B06D20+11B	call    sub_140B06D20
            //	p	sub_140BC9070+10B	call    sub_140BC9070 AE
            REL::Relocation<std::uintptr_t> target{REL::RelocationID(63030, 63948), REL::Relocate(0x11B, 0x10B)};
            if (!VerifyCallSite(target.address(), REL::RelocationID(63030, 63948), "LoadBehaviorGraph"))
                return;
            auto &trampoline = SKSE::GetTrampoline();
            _Load = trampoline.write_call<5>(target.address(), Load);
        }
        private:
        static RE::hkbBehaviorGraph* Load(const char *a_folderRoot, const char *a_behaviorFilePath, uint64_t a_assetLoader, void* a_unk04);
        static inline REL::Relocation<decltype(Load)> _Load;
    };


    // Since we don't know the project folder paths, we hook project load to map them
    // Up	p	BShkbAnimationGraph__sub_140AEDD10+108	call    sub_140AFFFE0
    //	p	sub_140BB0800:loc_140BB0911	call    sub_140BC1B10
    class LoadBehaviorProjectHook
    {
        public:
        static void Install()
        {
            REL::Relocation<std::uintptr_t> target{ REL::RelocationID(62640, 63585), REL::Relocate(0x108, 0x111) };
            if (!VerifyCallSite(target.address(), REL::RelocationID(62921, 63844), "LoadBehaviorProject"))
                return;
            auto &trampoline = SKSE::GetTrampoline();
            _LoadBehaviorProject = trampoline.write_call<5>(target.address(), LoadBehaviorProject);
        }
        private:
        static RE::hkbBehaviorGraph* LoadBehaviorProject(const char* folder, const char* name, RE::hkbCharacter& hchar, uint64_t project_data_handle, RE::BSFixedString& filename, RE::BSScrapArray<RE::hkbBehaviorGraph*>& hgraphs);
        static inline REL::Relocation<decltype(LoadBehaviorProject)> _LoadBehaviorProject;
    };

    

    /*Up	p	AnimationFileManagerSingleton__Func2_140B0A150+B6	call    AnimationFileManagerSingleton__sub_140B0A5C0
    	p	AnimationFileManagerSingleton__Func1_140B09FB0+89	call    AnimationFileManagerSingleton__sub_140B0A5C0*/
        // Up	p	sub_140BCBB90:loc_140BCBC1B	call    sub_140BCC220
   class ValidClipHook
   {

    public:
    static void Install()
    {
        REL::Relocation<std::uintptr_t> target{ REL::RelocationID(63069, 63981), REL::Relocate(0x89, 0x8B) };
        if (!VerifyCallSite(target.address(), REL::RelocationID(63075, 63987), "IsValidClip"))
            return;
        auto &trampoline = SKSE::GetTrampoline();
        _IsValidClip = trampoline.write_call<5>(target.address(), IsValidClip);
    }
    private:
    /*
    (unsigned __int8)AnimationFileManagerSingleton::sub_140B0A5C0(
                          a1,
                          (void *)a2,
                          *(_WORD *)&a3[1]._pad_14[4],
                          (BSSynchronizedClipGenerator *)a4,
                          &a2a,
                          &a6) 
    */
    static bool IsValidClip(RE::AnimationFileManagerSingleton* a_manager, const RE::hkbContext& a_context, uint16_t animationBindingIndex, RE::BSSynchronizedClipGenerator* a_synchronizedClipGenerator,
    void* unk05, uint32_t *a_outUserData);
    static inline REL::Relocation<decltype(IsValidClip)> _IsValidClip;
    static inline std::unordered_set<uint16_t> portedAnimationIndices; // we don't need a lock because Converter already throttles
   };


    static void InstallHooks()
    {
        SKSE::AllocTrampoline(14 * 3); 
        ValidClipHook::Install();
        LoadBehaviorGraphHook::Install();
        LoadBehaviorProjectHook::Install();
    }
}