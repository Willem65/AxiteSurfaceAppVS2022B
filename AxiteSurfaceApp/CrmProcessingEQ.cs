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
        private void buttonsCRMRProcessingEQ_MouseDown(object sender, MouseEventArgs e)
        {
            upDownProc(sender, 1);
        }

        private void buttonsCRMRProcessingEQ_MouseUp(object sender, MouseEventArgs e)
        {
            upDownProc(sender, 0);
        }

        private static void upDownProc(object sender, int upDwn)
        {
            Button b = (Button)sender;
            string buttonNumber = b.Name.Substring(18, 2);
            int buttonNum = Convert.ToInt32(buttonNumber);
            sensor(1201 + buttonNum, upDwn);
        }

        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        //-------------------------- Start Recieving Actuators -buttonProcessingEq01--------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------

        private void ActCRMProcessingEQ(int obje, string Val)
        {
            int intVal = 0;

            if (obje > 1036)
            {
                intVal = Convert.ToInt32(Val);
            }
            // ---------------------------- MODULE 1 SWITCHES -------------------------------------------            

            for (int p = 0; p < 13; p++)
            {
                if (obje == 1201 + p)
                {
                    // Find button by name                    
                    Button buttonByName = Controls.Find("buttonProcessingEq" + p.ToString("D2"), true).FirstOrDefault() as Button;

                    if (buttonByName != null)
                    {
                        switch (intVal)
                        {
                            case 0:
                                buttonByName.BackColor = Color.White;
                                break;
                            case 1:
                                buttonByName.BackColor = Color.LightGreen;
                                break;
                            case 2:
                                buttonByName.BackColor = Color.Tomato;
                                break;
                            case 3:
                                buttonByName.BackColor = Color.Yellow;
                                break;
                        }
                    }
                }
            }
        }
    }
}