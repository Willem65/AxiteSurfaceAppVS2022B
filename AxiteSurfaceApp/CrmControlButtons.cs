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
        //--------------------------- buttonControl01 -------------------------------------------------
        private void ControlButtons_MouseDown(object sender, MouseEventArgs e)
        {
            UpDownCtrl(sender, 1);
        }

        private void ControlButtons_MouseUp(object sender, MouseEventArgs e)
        {
            UpDownCtrl(sender, 0);
        }

        private static void UpDownCtrl(object sender, int v)
        {
            Button b = (Button) sender;
            string buttonNumber = b.Name.Substring(13, 2);
            int buttonNum = Convert.ToInt32(buttonNumber);
            sensor(1213 + buttonNum, v);
        }


        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        //-------------------------- Start Recieving Actuators ---------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        private void ActCRMcontrol(int obje, string Val)
        {
            {
                int intVal = 0;

                if (obje > 1036)
                {
                    intVal = Convert.ToInt32(Val);
                }
                // ---------------------------- MODULE 1 SWITCHES -------------------------------------------            

                for (int p = 0; p < 17; p++)
                {
                    if (obje == 1213 + p)
                    {
                        // Find button by name                    
                        Button buttonByName = Controls.Find("buttonControl" + p.ToString("D2"), true).FirstOrDefault() as Button;

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
            }
        }
    }
}