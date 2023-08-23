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
using System.Threading.Tasks;
using System.Windows.Forms;

namespace AxiteSurfaceApp
{
    public partial class ChannelsForm : Form
    {
        static int oldtst, newtst;

        public async Task VuMeterAsync(int intVal, string boxNr)
        {
            // Find the picture box control in the form's Controls collection
            PictureBox pictureBox = (PictureBox)this.Controls.Find("pictureBox" + boxNr, true).FirstOrDefault();

            // Check if the picture box is found
            if (pictureBox != null)
            {
                if (oldtst > 0)
                {
                    oldtst = oldtst - 3;
                }

                if (oldtst <= intVal)  // up edge reaction
                {
                    oldtst = intVal;
                }

                //Debug.Print(intVal.ToString());
                intVal = CorrTabel((double)oldtst);
                await ChangePictureBoxWidthAsync(pictureBox, intVal - 4);
            }           
        }


        public async Task ChangePictureBoxWidthAsync(PictureBox pictureBox, double newWidth)
        {

            await Task.Run(() =>
            {
                // Run the code in a separate thread to avoid blocking the UI thread
                // Update the width of the PictureBox
                pictureBox.Invoke((MethodInvoker)(() =>
                {
                    newtst = (int)newWidth;

                    pictureBox.Width = newtst;
                }));
            });
        }
        //---------------------------------------------------------------------------------------------

        public int CorrTabel(double tst)
        {
            double rtst = 0;


            bool checkBoxState = false;
            if (checkBoxState == true)
            {
                //Debug.Print("test");
                //rtst = tst*29;
                rtst = Math.Log10((double)tst) * 29;
            }
            else
            {

                if (tst > 147) rtst = 40;
                else if (tst > 146) rtst = 39;
                else if (tst > 145) rtst = 38;
                else if (tst > 144) rtst = 36;
                else if (tst > 143) rtst = 34;
                else if (tst > 142) rtst = 33;
                else if (tst > 141) rtst = 32;   //7
                else if (tst > 140) rtst = 31;
                else if (tst > 139) rtst = 30;
                else if (tst > 138) rtst = 29;
                else if (tst > 137) rtst = 28;
                else if (tst > 136) rtst = 27;   //6
                else if (tst > 136) rtst = 26;
                else if (tst > 135) rtst = 25;
                else if (tst > 134) rtst = 24;
                else if (tst > 133) rtst = 23;   //4
                else if (tst > 132) rtst = 22;
                else if (tst > 131) rtst = 21;
                else if (tst > 130) rtst = 19;
                else if (tst > 129) rtst = 18;   //5 
                else if (tst > 127) rtst = 16;
                else if (tst > 125) rtst = 14;   //5
                else if (tst > 123) rtst = 13;
                else if (tst > 121) rtst = 12;
                else if (tst > 120) rtst = 11;
                else if (tst > 119) rtst = 10;   //4
                else if (tst > 117) rtst = 9;
                else if (tst > 116) rtst = 8;
                else if (tst > 114) rtst = 7;
                else if (tst > 111) rtst = 6;
                else if (tst > 103) rtst = 5;
                else if (tst > 92) rtst = 4;
                else if (tst > 76) rtst = 3;
                else if (tst > 57) rtst = 2;
                else if (tst >= 35) rtst = 1;
                else if (tst < 17) rtst = 0;
            }
            return (int)rtst;
        }

    }




}

