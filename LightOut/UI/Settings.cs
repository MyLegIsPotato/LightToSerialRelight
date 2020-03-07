using BeatSaberMarkupLanguage.Attributes;
using BS_Utils.Utilities;
using System;
using System.Collections.Generic;
using System.Linq;
using System.IO.Ports;
using TMPro;

namespace LightOut
{
    class Settings : PersistentSingleton<Settings>
    {
        public static Config config;
        public static SerialPort arduinoPort;
        public static Settings settings;

        [UIValue("boolEnable")]
        public bool _isModEnabled
        {
            get => config.GetBool("LightOut", nameof(_isModEnabled), true, true);
            set => config.SetBool("LightOut", nameof(_isModEnabled), value);
        }

        [UIValue("list-options")]
        private List<object> options = SerialPort.GetPortNames().ToList<object>();

        [UIValue("list-choice")]
        public string listChoice
        {
            get => config.GetString("LightOut", nameof(listChoice), "");
            set => config.SetString("LightOut", nameof(listChoice), value);
        }

        [UIValue("baud-options")]
        private List<object> rates = new object[] { 1200, 2400, 4800, 9600, 19200, 38400, 57600, 74880, 115200, 230400, 250000, 500000, 1000000 }.ToList<object>();

        [UIValue("baud-choice")]
        public int baudChoice
        {
            get => config.GetInt("LightOut", nameof(baudChoice), 1200);
            set => config.SetInt("LightOut", nameof(baudChoice), value);
        }

        [UIAction("#apply")]
        public void UpdateConnection()
        {
            if (_isModEnabled)
            {
                OpenConnection();
            }
            else
            {
                CloseConnection();
            }
        }
        public void OpenConnection()
        {
            Logger.log.Info("Connecting...");
            try
            {
                arduinoPort = new SerialPort(listChoice, baudChoice, Parity.None, 8);
                arduinoPort.Open();
                if (arduinoPort.IsOpen) //PUT ARDUINO INTO STANDBY MODE
                {
                    Logger.log.Notice("Connecting succesful.");
                    arduinoPort.Write("r");
                    arduinoPort.Write("#");
                }
            }
            catch (Exception e)
            {
                CloseConnection();
                Logger.log.Error("Connecting failed.");
                Logger.log.Error(e);
            }

        }

        //MODAL

        [UIComponent("modified-text")]
        private TextMeshProUGUI modifiedText;

        [UIAction("refresh-btn-action")]
        private void RefreshStatus()
        {
            if (arduinoPort.IsOpen)
            {
                modifiedText.text = "Connection with arduino has been established.";
            }
            else
            {
                modifiedText.text = "There was some error connecting to arduino.";
            }
        }

        public void Awake()
        {
            config = new Config("LightOut");
        }
        public void CloseConnection()
        {
            if (arduinoPort.IsOpen)
            {
                Logger.log.Notice("Disconnecting...");
                try
                {
                    arduinoPort.Write("@");
                    arduinoPort.Close();
                    Logger.log.Notice("Disconnecting succesful.");
                }
                catch (Exception e)
                {
                    throw e;
                }

            }

        }
    }
}
