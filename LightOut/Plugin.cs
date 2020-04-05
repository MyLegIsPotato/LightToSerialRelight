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
using IPA;
using IPALogger = IPA.Logging.Logger;
using System.IO.Ports;
using BS_Utils;
using BS_Utils.Utilities;

namespace LightOut
{
    [Plugin(RuntimeOptions.DynamicInit)]
    public class Plugin
    {
        public string Name => "LightToSerialRelight";
        public string Version => "1.8.1";

        internal static bool gameCoreJustLoaded = false;
        public static IPA.Logging.Logger log;
        public bool eventAdded = false;
        public static Plugin thisPlugin;

        [Init]
        public Plugin(IPA.Logging.Logger logger)
        {
            log = logger;
            log.Info("Initialized LightToSerialRelight");
            thisPlugin = this;
        }

        [OnStart]
        public void OnStart()
        {
            log.Info("Starting LightToSerialRelight");
            BSEvents.menuSceneLoaded += BSEvents_menuSceneLoaded;
            BSMLSettings.instance.AddSettingsMenu("LightOut", "LightOut.UI.settings.bsml", Settings.instance);
        }

        public void BSEvents_menuSceneLoaded()
        {
           Plugin.log.Info("Loaded Menu");
           if (Settings.instance._isModEnabled && Settings.arduinoPort.IsOpen && !eventAdded) //Only happens once per game restart if mod is always enabled.
           {
                log.Info("Adding event for gameSceneLoaded.  Level Event Listener will be spawned when starting a level.");
                BS_Utils.Utilities.BSEvents.gameSceneLoaded += AddEventListener;
                eventAdded = true;
           }else if(eventAdded && (!Settings.instance._isModEnabled || !Settings.arduinoPort.IsOpen))
            {
                log.Info("Removing event for gameSceneLoaded. Level Event Listener won't be spawned anymore.");
                BS_Utils.Utilities.BSEvents.gameSceneLoaded -= AddEventListener;
                eventAdded = false;
           }

        }

        [OnExit]
        public void OnExit()
        {
            Settings.instance.CloseConnection();
        }

        void AddEventListener()
        {
            new GameObject("EventListener").AddComponent<EventListener>();
        }


    }
}