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
        public static SerialPort arduinoPort = null;
        public static Settings settings;


        [UIValue("boolEnable")]
        public bool _isModEnabled
        {
            get => config.GetBool("LightOut", "_isModEnabled", true, true);
            set => config.SetBool("LightOut", "_isModEnabled", value);
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

        [UIValue("rainbowMode")]
        public bool rainbowMode
        {
            get => config.GetBool("LightOut", "rainbowMode", true, true);
            set => config.SetBool("LightOut", "rainbowMode", value);
        }

        [UIAction("#apply")]
        public void UpdateConnection()
        {
            OpenConnection();
        }
        public void OpenConnection()
        {
            if(arduinoPort != null) //is instantiated
            {
                if (arduinoPort.IsOpen)
                {
                    CloseConnection();
                    OpenConnection();
                    Connect();
                }
            }
            else
            {
                arduinoPort = new SerialPort(listChoice, baudChoice, Parity.None, 8);
                Connect();
            }
        }

        public void Connect()
        {
            try
            {
                arduinoPort.Open();
                arduinoPort.ReadTimeout = 3000;
                byte[] x = new byte[1] { 69 };
                arduinoPort.Write(x, 0, 1);
                int incomingByte = arduinoPort.ReadByte();
                Logger.log.Notice("Connecting succesful.");
                arduinoPort.Write("r");
                arduinoPort.Write("#");
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
            if(arduinoPort != null)
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
}
