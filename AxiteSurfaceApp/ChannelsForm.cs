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
        Process process = new Process();

        MMDevice device;

        //int vuLevel1 = 0;
        //bool checkBoxState = false;

        bool debugList = false;
        bool MotorFader = false;  // debugList = checkBox2.Checked;
        bool connected = false, enableVU = true;
        int Tinterval1;

        int /*oldtst1 = 0, oldtst2 = 0,  eventTeller, */  eventTeller2; // eventTeller3, eventTeller4;

        public ChannelsForm()
        {
            InitializeComponent();
            buttonNotVisable();

            NAudio.CoreAudioApi.MMDeviceEnumerator enumerator = new NAudio.CoreAudioApi.MMDeviceEnumerator();

            var devices = enumerator.EnumerateAudioEndPoints(NAudio.CoreAudioApi.DataFlow.All, DeviceState.Active);

            comboBox2.Items.AddRange(devices.ToArray());

            label5.Text = timer1.Interval.ToString();

        }

        #region

        private void buttonNotVisable()
        {
            comboBox2.Visible = false;
            checkBox4debugListVU.Visible = false;
            //button4Source.Visible = false;
            //button4.Visible = false;
            button2.Visible = false;
            button7.Visible = false;
            //button5.Visible = false;
            button6.Visible = false;
            button8.Visible = false;
            // button1.Visible = false;
            //StartButton.Visible = false;
            checkBox2.Visible = false;
            checkBox3.Visible = false;
            label5.Visible = false;
            label6.Visible = false;
        }

        private void buttonVisable()
        {
            comboBox2.Visible = true;
            checkBox4debugListVU.Visible = true;
            //button4Source.Visible = true;
            //button4.Visible = true;
            button2.Visible = true;
            button7.Visible = true;
            //button5.Visible = true;
            button6.Visible = true;
            button8.Visible = true;
            // button1.Visible = true;
            //StartButton.Visible = true;
            checkBox2.Visible = true;
            checkBox3.Visible = true;
            label5.Visible = true;
            label6.Visible = true;
        }

        // Load Button Switches Value
        private void faderCtrl1_Load(object sender, EventArgs e)
        {
            faderCtrl1.Value = 0;

        }

        private void Channel1Form_FormClosing(Object sender, FormClosingEventArgs e)
        {
            try
            {
                if (!process.WaitForExit(500))
                {
                    process.Kill();
                }
            }
            catch (Exception ex)
            {
                Debug.WriteLine(ex);
            }
        }



        private void Channel1Form_Load(object sender, EventArgs e)
        {


        }


        //---------------------- Select Source---------------------------------------------
        //private void button4Source_MouseDown(object sender, MouseEventArgs e)
        //{
        //    //enableVU = false;
        //    sensor(1195, 1);
        //}
        //private void button4Source_MouseUp(object sender, MouseEventArgs e)
        //{
        //    //enableVU = false;
        //    sensor(1195, 0);
        //}

        private void StartButton_Click(object sender, EventArgs e)
        {
            device.AudioEndpointVolume.VolumeStepDown();
        }

        public void button1_Click_1(object sender, EventArgs e)
        {
            device.AudioEndpointVolume.VolumeStepUp(); // = (float)(faderCtrl1.Value / 1024); //0.8f;
        }



        private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            try
            {
                if (comboBox1.SelectedIndex == 2)
                {
                    //device.AudioEndpointVolume.MasterVolumeLevelScalar = 0.5f;
                }
                if (comboBox2.SelectedItem != null)
                {
                    device = (MMDevice)comboBox2.SelectedItem;
                    checkBox3.Checked = true;

                    //monitorAction("01", "butPec", 1);
                    //monitorAction("01", "butPec", 0);
                    //monitorAction("02", "butPec", 1);
                    //monitorAction("02", "butPec", 0);
                    //monitorAction("03", "butPec", 1);
                    //monitorAction("03", "butPec", 0);
                    //monitorAction("04", "butPec", 1);
                    //monitorAction("04", "butPec", 0);

                    //monitorAction("01", "butMec", 1);
                    //monitorAction("01", "butMec", 0);
                    //monitorAction("02", "butMec", 1);
                    //monitorAction("02", "butMec", 0);
                    //monitorAction("03", "butMec", 1);
                    //monitorAction("03", "butMec", 0);
                    //monitorAction("04", "butMec", 1);
                    //monitorAction("04", "butMec", 0);

                    timer2.Enabled = true;

                }
            }
            catch (Exception)
            {
                MessageBox.Show("Choose a soundcard device or connect to the engine");
            }
        }




        // ################################ TIMER 2 TICK ###################################################
        public void timer2_Tick(object sender, EventArgs e)
        {
            if (device != null && enableVU == true)
            {
                if (eventTeller2 == 0)
                {
                    eventTeller2++;
                    ushort vuLevel1 = ((ushort)(device.AudioMeterInformation.PeakValues[0] * 1023));
                    sensor(1088, vuLevel1);  // Stuur de audio peak value naar de fader
                    ushort vuLevell1 = (ushort)(device.AudioMeterInformation.PeakValues[1] * 1023);
                    sensor(1089, vuLevell1);
                    //Thread.Sleep(5);

                }


                else if (eventTeller2 == 1)
                {
                    //ushort vuLevel1 = (ushort)(device.AudioMeterInformation.PeakValues[0] * 1023);
                    //sensor(1087, vuLevel1);  // Stuur de audio peak value naar de fader
                    eventTeller2 = 0;
                    //Thread.Sleep(5);

                }

            }
        }



        // ################################ TIMER 1 TICK ###################################################
        //string acc = "#0000_0000";
        //int oldVal=0;
        int obje = 0;
        string Val;

        private void timer1_Tick_1(object sender, EventArgs e)
        {
            // StringBuilder ObjectActuatorStr = new StringBuilder(capacity: 11261);   // 209/261 minimum
            myRunning obj = new myRunning();
            obj.Running(Callback);
        }

        void Callback(string i)
        {
            string acc = i;
            // Check for legal value
            if (acc.Length < 10) // && acc.Length > 0)
                acc = "#0000_0000";

            //  

            Regex myRegex = new Regex("#", RegexOptions.RightToLeft);
            //Regex myRegex = new Regex(@"^#[0-9999]$_[0-9999]");
            //Regex myRegex = new Regex(@"^#\d{4}^_\d{4}^#\d{4}^_\d{4}");
            // Match the regular expression pattern against a text string.
            Match myMatch = myRegex.Match(acc.ToString());



            do
            {
                obje = Convert.ToInt32(acc.Substring(myMatch.Index + 1, 4));

                //if (acc.Length > 9 && obje > 1035)
                //{
                //    Val = Convert.ToInt32(acc.Substring(myMatch.Index + 6, 4));
                //}

                if (acc.Length > 9 && obje > 1035)
                {
                    Val = acc.Substring(myMatch.Index + 6, 4);
                }
                else if (acc.Length > 9 && obje < 1036 && obje > 1023)
                {
                    Val = acc.Substring(myMatch.Index + 6, 8);
                }

                //Debug.Print(intVal.ToString());
                //if (Val != null)
                //    Debug.Print(acc.ToString());

                ActColorState(obje, Val);
                ActCRMrouting(obje, Val);
                ActCRMProcessingEQ(obje, Val);
                ActCRMmonitor(obje, Val);
                ActCRMcontrol(obje, Val);

                if (debugListVU == true)
                {
                    if (obje >= 1036 && obje <= 1047)
                    {
                        listBox1.Items.Add(obje.ToString() + " " + Val.ToString());
                        listBox1.SelectedIndex = listBox1.Items.Count - 1;
                    }
                }

                else if (debugList == true && Val != null && obje >= 1048)
                {
                    listBox1.Items.Add(obje.ToString() + " " + Val.ToString());
                    listBox1.SelectedIndex = listBox1.Items.Count - 1;
                }


            } while ((myMatch = myMatch.NextMatch()).Success);

            //  
        }
        // ################################ END TIMER 1 ###################################################





        [DllImport("mbn.dll", CharSet = CharSet.Ansi, EntryPoint = "sensor@8")] // CharSet = CharSet.Ansi, EntryPoint = "mbnVersion@0", ExactSpelling = false, CallingConvention = CallingConvention.Cdecl)]
        public static extern void sensor(int obj, int num);

        [DllImport("mbn.dll", CharSet = CharSet.Ansi, EntryPoint = "connectToObjects@16")]
        public static extern void connectToObjects(int A, int B, int C, int D);

        private void ConnectButton_Click(object sender, EventArgs e)
        {
            string ip = comboBox1.Text;

            int ipA = 192, ipB = 168, ipC = 0, ipD = 0;

            for (int Tel = 0; Tel < 4; Tel++)
            {
                string ipOut = ip.Split('.')[Tel];
                if (ipOut == "")
                {
                    MessageBox.Show("Wrong or missing IP address ");
                    return;
                }
                if (Tel == 0) ipA = Convert.ToInt32(ipOut);
                if (Tel == 1) ipB = Convert.ToInt32(ipOut);
                if (Tel == 2) ipC = Convert.ToInt32(ipOut);
                if (Tel == 3) ipD = Convert.ToInt32(ipOut);
            }
            if (connected == false)
            {
                connected = true;
                connectToObjects(ipA, ipB, ipC, ipD);
                listBox1.Items.Add("Connected !");
                buttonVisable();
            }
            else
            {
                MessageBox.Show("for an new or different connection, close and start the application now ");
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            timer1.Interval = 1;
            //timer1.Start();
        }

        private void button7_Click(object sender, EventArgs e)
        {
            Tinterval1 = 200;
            timer1.Interval = Tinterval1;
            label5.Text = Tinterval1.ToString();

        }

        private void button6_Click_1(object sender, EventArgs e)
        {
            if (Tinterval1 < 11)
                Tinterval1 = Tinterval1 + 1;
            else if (Tinterval1 < 101)
                Tinterval1 = Tinterval1 + 10;
            else if (Tinterval1 > 99)
                Tinterval1 = Tinterval1 + 100;

            if (Tinterval1 >= 10000)
                Tinterval1 = 10000;

            timer1.Interval = Tinterval1;
            label5.Text = Tinterval1.ToString();
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            debugList = checkBox2.Checked;
        }

        //private void checkBox1_CheckedChanged(object sender, EventArgs e)
        //{
        //    checkBoxState = checkBox1.Checked;
        //}

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            MotorFader = checkBox3.Checked;
        }

        private void label5_Click(object sender, EventArgs e)
        {

        }

        private void button8_Click(object sender, EventArgs e)
        {
            if (Tinterval1 < 101)
                Tinterval1 = Tinterval1 - 10;
            else if (Tinterval1 <= 10000)
                Tinterval1 = Tinterval1 - 100;

            if (Tinterval1 <= 0)
                Tinterval1 = 1;

            timer1.Interval = Tinterval1;
            label5.Text = Tinterval1.ToString();
        }




        bool debugListVU = false;



        private void checkBox4debugListVU_CheckedChanged(object sender, EventArgs e)
        {
            debugListVU = checkBox4debugListVU.Checked;
        }


        private void button6_MouseDown(object sender, MouseEventArgs e)
        {
            sensor(1030, 1);

        }
        private void button6_MouseUp(object sender, MouseEventArgs e)
        {
            sensor(1030, 0);

        }


        private void button4_Click(object sender, EventArgs e)
        {
            timer2.Start();
        }

        private void button5_Click(object sender, EventArgs e)
        {
            timer2.Stop();
        }





        private void button9_Click(object sender, EventArgs e)
        {
            myRunning obj = new myRunning();
            obj.Running(Callback);
        }


        #endregion

        private void colorProgressBar1_Click(object sender, EventArgs e)
        {

        }



        private void label3_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {

        }

        private void label2_Click(object sender, EventArgs e)
        {

        }


        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void buttonHideRisize_Click(object sender, EventArgs e)
        {
            ChannelsForm.ActiveForm.Height = 888;
            ChannelsForm.ActiveForm.Width = 970;
            comboBox1.Visible = false;
            ConnectButton.Visible = false;

        }

        int screwClicked;

        private void pictureBox3_Click(object sender, EventArgs e)
        {
            screwClicked++;
            hideDebugTools();
        }

        

        private void hideDebugTools()
        {
            if (screwClicked == 1)
            {   if (surfaceCheckBox.Checked == false)
                {
                    ChannelsForm.ActiveForm.Height = 888;
                    ChannelsForm.ActiveForm.Width = 970;
                    comboBox1.Visible = false;
                    ConnectButton.Visible = false;
                    surfaceNr.Visible = false;
                    labelSurface.Visible = false;
                    surfaceCheckBox.Visible = false;
                }
                else
                {
                    ChannelsForm.ActiveForm.Height = 888;
                    ChannelsForm.ActiveForm.Width = 644;
                    comboBox1.Visible = false;
                    ConnectButton.Visible = false;
                    surfaceNr.Visible = false;
                    labelSurface.Visible = false;
                    surfaceCheckBox.Visible = false;
                }
            }
            else if (screwClicked == 2)
            {
                screwClicked = 0;
                ChannelsForm.ActiveForm.Height = 888;
                ChannelsForm.ActiveForm.Width = 1400;
                comboBox1.Visible = true;
                ConnectButton.Visible = true;
                surfaceNr.Visible = true;
                labelSurface.Visible = true;
                surfaceCheckBox.Visible = true;
            }

        }



        private void label2_3_Click(object sender, EventArgs e)
        {

        }



        private void label13_Click(object sender, EventArgs e)
        {

        }




        private void progressBar1_Click(object sender, EventArgs e)
        {

        }

        private void button1_Click_2(object sender, EventArgs e)
        {
            timer2.Enabled = false;
        }



        private void timer3_Tick(object sender, EventArgs e)
        {

        }


    }



    public class myRunning
    {

        [DllImport("mbn.dll", CharSet = CharSet.Ansi, EntryPoint = "retString@4")]
        public static extern int retString(StringBuilder ObjectActuatorStr);

        StringBuilder ObjectActuatorStr = new StringBuilder(capacity: 0x10240);


        public delegate void CallBack(string str);

        public void Running(CallBack obj)
        {

            retString(ObjectActuatorStr);

            obj(ObjectActuatorStr.ToString());

        }

    }
}
