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

        // ----------------------- MODULES -Send to Engine ---------------------------------------------
        private void button1_1_MouseDown(object sender, MouseEventArgs e)
        {
            Button b = (Button)sender;
            string buttonModule = b.Name;
            string buttonText = b.Text;
            buttonAction(buttonModule, buttonText, 1);
        }

        private void button1_1_MouseUp(object sender, MouseEventArgs e)
        {
            Button b = (Button)sender;
            string buttonModule = b.Name;
            string buttonText = b.Text;
            buttonAction(buttonModule, buttonText, 0);
        }
                                  
        private void buttonAction(string module, string bTxt, int UpDown)
        {
            int m = Convert.ToInt32(module.Substring(6, 1))-1;
            if (bTxt == "B")
                sensor(1060+m, UpDown);
            else if (bTxt == "TB")
                sensor(1066+m, UpDown);
            else if (bTxt == "Cue")
                sensor(1072+m, UpDown);
            else if (bTxt == "Start")
                sensor(1078+m, UpDown);
            else if (bTxt == "-")  
                sensor(1048 + m, UpDown-1);  //<---------------------- Contact encoder min
            else if (bTxt == "+")
                sensor(1048 + m, UpDown);    //<---------------------- Contact encoder plus
            else if (bTxt == "o")
                sensor(1054 + m, UpDown);    //<---------------------- Contact encoders switch
        }


        //---------------------- Faders ------------------------------------------------------------
        private void picSliders_ValueChanged(object sender, System.EventArgs e)
        {
            CustomFader.FaderCtrl f = (CustomFader.FaderCtrl)sender;
            string faderModule = f.Name;
            int m = Convert.ToInt32(faderModule.Substring(9, 1))- 1;
            sensor(1084 + m, f.Value);
        }
        //-------------------------- END sending Sensors---------------------------------------------------------------------------------------------------





        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        //-------------------------- Start Recieving Actuators ---------------------------------------------------------------------------------------------------
        //--------------------------------------------------------------------------------------------------------------------------------------------------------
        private void ActColorState(int obje, string Val)
        {
            
            int intVal = 0;

            if (obje > 1035)
                intVal = Convert.ToInt32(Val);
            //Debug.Print(intVal.ToString());
            //if (Val != null) Debug.Print(Val.ToString());



            // ---------------------------- Oled Display Text -------------------------------------------
            if (obje <= 1035 && obje >= 1024)
            {

                for (int t = 1; t < 7; t++)  // Alle modules
                {



                    // Find label by name module nr
                    var displayByName = Controls.Find("labelModule" + t, true).FirstOrDefault();

                    if (displayByName != null && obje == 1024 + t - 1)
                        displayByName.Text = Val.Substring(5, 2);



                    // Find label by name module nr 
                    var displayByNameMod = Controls.Find("label" + t + "_" + 2, true).FirstOrDefault();

                        if (displayByNameMod != null && obje == 1024 + t - 1)
                            displayByNameMod.Text = Val;

                    // Find label by name
                    var displayByNameSrc = Controls.Find("label" + t + "_" + 3, true).FirstOrDefault();

                        if (displayByNameSrc != null && obje == 1030 + t - 1)
                            displayByNameSrc.Text = Val;

                }
            }


            // ---------------------------- Oled Display Labels ---------------------------------
            // Wacht totdat het object langs komt
            // -------------------------------------------------------------------------- 

            else if (obje >= 1138 && obje <= 1191)
            {
                for (int m = 0; m < 7; m++)  // Alle modules
                {
                    // Ga alle labels af, doe dit 9 keer
                    for (int t = 0; t < 9; t++)
                    {
                        // Find label by name
                        Control displayByName = Controls.Find("displayLabel" + m.ToString() + "_" + t.ToString(), true).FirstOrDefault();

                        if (displayByName != null)
                        {
                            // Gevonden label voor het betreffende object
                            int temp = (1138 - 6) + (t * 6) + (m - 1);
                            if (obje == temp)
                            {
                                if (Convert.ToInt32(Val) == 1)
                                {
                                    displayByName.Text = displayByName.Text;
                                    displayByName.ForeColor = Color.White;

                                }
                                else if (Convert.ToInt32(Val) == 0)
                                {
                                    displayByName.Text = displayByName.Text;
                                    displayByName.ForeColor = Color.Black;
                                }
                            }
                        }
                    }
                }
            }

            // ---------------------------- MODULE MOTOR FADERS -------------------------------------------
            else if (obje >= 1084 && obje <= 1090)
            {


                for (int m = 1; m < 7; m++)
                {
                    var dispByNameDBu = Controls.Find("label" + m + "_" + 2, true).FirstOrDefault();

                    if (obje == 1084 + m - 1)
                        dispByNameDBu.Text = ((double)((1023 - intVal) / 20)).ToString() + " dBu";
                }
                //else if (obje == 1085)
                //    label1_2.Text = intVal.ToString();
                //else if (obje == 1086)
                //    faderCtrl3.Value = intVal;
                //else if (obje == 1087)
                //    faderCtrl4.Value = intVal;
                //else if (obje == 1088)
                //    faderCtrl5.Value = intVal;
                //else if (obje == 1089)
                //    faderCtrl6.Value = intVal;

                if (intVal > 1023) intVal = 1024;
                if (intVal < 1) intVal = 0;

                if (MotorFader == true)
                    {
                        if (intVal > 1023) intVal = 1024;
                        if (intVal < 1) intVal = 0;

                        if (obje == 1084)
                            faderCtrl1.Value = intVal;
                        else if (obje == 1085)
                            faderCtrl2.Value = intVal;
                        else if (obje == 1086)
                            faderCtrl3.Value = intVal;
                        else if (obje == 1087)
                            faderCtrl4.Value = intVal;
                        else if (obje == 1088)
                            faderCtrl5.Value = intVal;
                        else if (obje == 1089)
                            faderCtrl6.Value = intVal;
                        
                    }
                
            }

            // ---------------------------- MODULE VU METERS -------------------------------------------

            var modnr = 0;

            for (var m = 1; m < 7; m = m + 1) // Alle modules
            {
                for (var v = 0; v < 2; v++)   // VU meter links en rechts
                    if (obje == 1036 + modnr + v)
                        VuMeterAsync(intVal, m + "_" + (v + 1));

                modnr = modnr + 2;
            }

            // ---------------------------- MODULE BUTTONS -------------------------------------------
            for (var m = 0; m < 7; m++)
            {
                for (var k = 0; k <= 4; k++)
                {
                    // Find button by name
                    if (obje == 1060 + m - 1 + (k - 1) * 6)
                    {
                        var buttonByName = Controls.Find("button" + m + "_" + k, true).FirstOrDefault() as Button;

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