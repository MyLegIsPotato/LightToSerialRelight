﻿using BeatSaberMarkupLanguage.Attributes;
using BS_Utils.Utilities;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.IO.Ports;
using TMPro;
using UnityEngine;

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

        [UIValue("event-options")]
        private List<object> events = new object[] {"noteCuts", "lightEvents"}.ToList<object>();

        [UIValue("event-choice")]
        public string eventChoice
        {
            get => config.GetString("LightOut", "eventChoice", "");
            set => config.SetString("LightOut", "eventChoice", value);
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
            if (_isModEnabled)
            {
                OpenConnection();
            }
            else
            {
                if (arduinoPort != null && arduinoPort.IsOpen)
                {
                    CloseConnection();
                }
            }

            Plugin.thisPlugin.BSEvents_menuSceneLoaded();
        }
        public void OpenConnection()
        {
            if(arduinoPort != null) //is instantiated
            {
                CloseConnection();
                if (!arduinoPort.IsOpen)
                {
                    StartCoroutine(Connect());
                }
            }
            else
            {
                arduinoPort = new SerialPort(listChoice, baudChoice, Parity.None, 8);
                StartCoroutine(Connect());
            }
        }

        public IEnumerator Connect()
        {
            bool portIsOpen = false;
            int incomingByte = 0;

            try
            {
                arduinoPort.Open();
                arduinoPort.ReadTimeout = 3000;
                Plugin.log.Notice("Port opened successfully.");
                portIsOpen = true;
            }
            catch (Exception e)
            {
                CloseConnection();
                Plugin.log.Error("Couldn't open the port, check connection and settings.");
                Plugin.log.Error(e);
                portIsOpen = false;
            }

            if (portIsOpen)
            {
                char[] x = new char[1] {'-'};
                arduinoPort.Write(x, 0, 1);
                yield return new WaitForSeconds(3);
                incomingByte = arduinoPort.ReadByte();
                if (incomingByte == (int)'a')
                {
                    Plugin.log.Notice("Connection established.");
                    arduinoPort.Write("r");
                    arduinoPort.Write("#");
                }
                else
                {
                    Plugin.log.Error("There was some error in two-way communication.");
                }
            }
            yield return null;
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
                    Plugin.log.Notice("Disconnecting...");
                    try
                    {
                        arduinoPort.Write("@");
                        arduinoPort.Close();
                        Plugin.log.Notice("Disconnecting succesful.");
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
