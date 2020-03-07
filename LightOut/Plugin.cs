using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Media;
using UnityEngine;
using UnityEngine.SceneManagement;
using System.Reflection;
using BeatSaberMarkupLanguage.Settings;
using Harmony;
using IPA;
using IPALogger = IPA.Logging.Logger;
using System.IO.Ports;
using BS_Utils;
using BS_Utils.Utilities;

namespace LightOut
{
    public class Plugin : IBeatSaberPlugin
    {
        public string Name => "LightSerialOutput";
        public string Version => "1.7.3";

        internal static bool harmonyPatchesLoaded = false;
        internal static HarmonyInstance harmonyInstance = HarmonyInstance.Create("com.mylegispotato.BeatSaber.LightOut");


        internal static bool gameCoreJustLoaded = false;

        public void Init(IPALogger logger)
        {
            Logger.log = logger;
        }

        void IBeatSaberPlugin.OnApplicationStart()
        {

        }

        void IBeatSaberPlugin.OnApplicationQuit()
        {
            Settings.instance.CloseConnection();
        }

        void IBeatSaberPlugin.OnUpdate()
        {

        }

        void IBeatSaberPlugin.OnFixedUpdate()
        {

        }

        void IBeatSaberPlugin.OnSceneLoaded(Scene scene, LoadSceneMode sceneMode)
        {
            Logger.log.Info("Loaded Scene: " + scene.name);
            if (scene.name == "MenuCore" || scene.name == "GameCore")
            {
                if (!harmonyPatchesLoaded && Settings.instance._isModEnabled)
                {
                    Logger.log.Info("LightsOut is ENABLED!");
                    if(!Settings.arduinoPort.IsOpen && Settings.instance._isModEnabled)
                    {
                        Settings.instance.OpenConnection();
                    }
                    if (Settings.arduinoPort.IsOpen)
                    {
                        BS_Utils.Utilities.BSEvents.gameSceneLoaded += AddEventListener;
                    }
                }
                if (harmonyPatchesLoaded && !Settings.instance._isModEnabled)
                {
                    Logger.log.Info("LightsOut is DISABLED!");
                }
            }
        }

        void IBeatSaberPlugin.OnSceneUnloaded(Scene scene)
        {

        }

        void IBeatSaberPlugin.OnActiveSceneChanged(Scene prevScene, Scene nextScene)
        { 
            if (nextScene.name == "MenuViewControllers")
            {
                Logger.log.Info("Loading Settings.");
                BSMLSettings.instance.AddSettingsMenu("LightOut", "LightOut.UI.settings.bsml", Settings.instance);
            }

        }

        void AddEventListener()
        {
            new GameObject("EventListener").AddComponent<EventListener>();
        }


    }
}