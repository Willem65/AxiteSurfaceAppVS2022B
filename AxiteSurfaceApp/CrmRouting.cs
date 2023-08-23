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
        //----------------------------- CRMRRouting ---------------------------------------------------------------------------------------------------------

        private void buttonsCRMRRouting_MouseDown(object sender, MouseEventArgs e)
        {
            UpDownCRM(sender, 1);
        }

        private void buttonsCRMRRouting_MouseUp(object sender, MouseEventArgs e)
        {
            UpDownCRM(sender, 0);
        }

        private static void UpDownCRM(object sender, int upDwn)
        {
            Button b = (Button)sender;
            string buttonNumber = b.Name.Substring(13, 2);
            int buttonNum = Convert.ToInt32(buttonNumber);
            sensor(1191 + buttonNum, upDwn);
        }

        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        //-------------------------- Start Recieving Actuators ---------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        private void ActCRMrouting(int obje, string Val)
        {
            int intVal = 0;

            if (obje > 1036)
            {
                intVal = Convert.ToInt32(Val);
            }
            // ---------------------------- MODULE 1 SWITCHES -------------------------------------------            
            NewMethod(obje, intVal);
        }

        private void NewMethod(int obje, int intVal)
        {            
            for (int p =0; p < 11; p++)
            {
                if (obje == 1191 + p)
                {

                    //Control displayByName = Controls.Find("buttonRouting" + p.ToString("D2") , true).FirstOrDefault();

                    // Find button by name                    
                    Button buttonByName = Controls.Find("buttonRouting" + p.ToString("D2"), true).FirstOrDefault() as Button;

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