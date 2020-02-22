using IPA.Utilities;
using Harmony;

namespace LightOut.HarmonyPatches
{
    [HarmonyPatch(typeof(BeatmapEventData))]
    [HarmonyPatch("Update")]
    class BeatmapEventDataPatch
    {
        static bool Prefix(BeatmapEventData __instance)
        {
            if (Plugin.gameCoreJustLoaded)
            {
                Plugin.gameCoreJustLoaded = false;
            }
            return false;
        }
    }
}
