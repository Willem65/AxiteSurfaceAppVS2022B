using NAudio.CoreAudioApi;
using System;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Windows.Forms;

namespace AxiteSurfaceApp
{
    public partial class ChannelsForm : Form
    {
        int levelMon=500;

        private void buttonCRMmonitor_MouseDown(object sender, MouseEventArgs e)
        {
            UpDownMon(sender, 1);
        }

        private void buttonCRMmonitor_MouseUp(object sender, MouseEventArgs e)
        {
            UpDownMon(sender, 0);
        }

        private void UpDownMon(object sender, int v)
        {
            Button b = (Button) sender;
            string buttonNumber = b.Name.Substring(1, 2);
            string buttonName = b.Name.Substring(3, 6);
            monitorAction(buttonNumber, buttonName, v);
        }


        private void monitorAction(string buttonNumber, string buttonName, int UpDown)
        {
            int m = Convert.ToInt32(buttonNumber) - 1;

            if (buttonName == "button")     //<---------- Button On/Off
                sensor(1230 + m, UpDown);
            else if (buttonName == "butMec")
            {
                EncoderTurn(m, -20);  //<---------- Contact encoder plus
            }
            else if (buttonName == "butPec")
            {
                EncoderTurn(m, 20);  //<---------- Contact encoder plus
            }
        }

        private void EncoderTurn(int m, int direction)
        {
            levelMon = levelMon + direction;
            if (levelMon >= 1023) levelMon = 1023;
            if (levelMon <= 0) levelMon = 0;
            sensor(1339 + m, levelMon); 
        }


        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        //-------------------------- Start Recieving Actuators ---------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        private void ActCRMmonitor(int obje, string Val)
        {
            int intVal = 0;

            if (obje > 1035)
                intVal = Convert.ToInt32(Val);

            //    // ---------------------------- MODULE 1 SWITCHES -------------------------------------------     
            for (int t = 0; t < 12; t++)
            {
                if (obje == 1230 + t - 1 )
                {
                    // Find button by name                    
                    Button buttonByName = Controls.Find("_" + t.ToString("D2") + "button", true).FirstOrDefault() as Button;

                    if (buttonByName != null)
                    {
                        if (intVal == 0)
                            buttonByName.BackColor = Color.White;
                        else if (intVal == 1)
                            buttonByName.BackColor = Color.LightGreen;
                        else if (intVal == 2)
                            buttonByName.BackColor = Color.Tomato;
                        else if (intVal == 3)
                            buttonByName.BackColor = Color.Yellow;
                    }
                }
            }
            for (int t = 0; t < 5; t++)
            {
                if (obje == 1339 + t - 1)
                {
                    // Find button by name Potentialmeters                 
                    Button buttonByName = Controls.Find("_" + t.ToString("D2") + "butPec", true).FirstOrDefault() as Button;

                    if (buttonByName != null)
                    {
                        double lv = Math.Round(intVal / 10.23, 0);
                        if (t == 1) labMpp1.Text = lv.ToString() + "%";
                        if (t == 2) labMpp2.Text = lv.ToString() + "%";
                        if (t == 3) labMpp3.Text = lv.ToString() + "%";
                        if (t == 4) labMpp4.Text = lv.ToString() + "%";
                    }
                }
            }
        }
    }
}