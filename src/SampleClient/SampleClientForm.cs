using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SampleClient
{
    public partial class SampleClientForm : Form
    {
        private const int LOG_LINE_MAX = 500;

        private int user_count = 0;
        private Dictionary<string, CUser> dic_user = new Dictionary<string,CUser>();
        private CUser current_user;

        public SampleClientForm()
        {
            InitializeComponent();
        }

        private void SampleClientForm_Load(object sender, EventArgs e)
        {

        }

        public void SetUserState(CUser user)
        {

            if (listView_User.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate()
                {
                    ListViewItem[] items = listView_User.Items.Find(user.user_name, false);
                    if (items.Length != 1)
                    {
                        Write("SetUserState Error");
                        return;
                    }
                        

                    items[0].SubItems[1].Text = GetStateString(user.State);
                    items[0].BackColor = GetStateColor(user.State);

                }));
            }
            else
            {
                ListViewItem[] items = listView_User.Items.Find(user.user_name, false);
                if (items.Length != 1)
                {
                    Write("SetUserState Error");
                    return;
                }

                items[0].SubItems[1].Text = GetStateString(user.State);
                items[0].BackColor = GetStateColor(user.State);

            }
        }

        private string GetStateString(CUser.ConnState state)
        {
            switch (state)
            {
                case CUser.ConnState.None:
                    return "none";
                case CUser.ConnState.Connecting:
                    return "connecting";
                case CUser.ConnState.Connected:
                    return "connected";
                default:
                    return "error";
            }
        }

        private Color GetStateColor(CUser.ConnState state)
        {
            switch(state)
            {
                case CUser.ConnState.None:
                    return Color.White;
                case CUser.ConnState.Connecting:
                    return Color.Yellow;
                case CUser.ConnState.Connected:
                    return Color.Green;
                default:
                    return Color.Red;
            }
        }

        // Write Log
        public void Write(string msg)
        {
            string logmsg = DateTime.Now.ToString("[HH:mm:ss] ") + msg;
            if (listBox_Log.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate()
                {
                    if (listBox_Log.Items.Count > LOG_LINE_MAX)
                        listBox_Log.Items.RemoveAt(0);

                    listBox_Log.Items.Add(logmsg);
                    listBox_Log.SelectedIndex = listBox_Log.Items.Count - 1;
                }));
            }
            else
            {
                if (listBox_Log.Items.Count > LOG_LINE_MAX)
                    listBox_Log.Items.RemoveAt(0);

                listBox_Log.Items.Add(logmsg);
                listBox_Log.SelectedIndex = listBox_Log.Items.Count - 1;
            }
        }


        // create
        private void button3_Click(object sender, EventArgs e)
        {
            int count = (int)numericUpDown1.Value;
            if(count < 1)
            {
                Write("invalid create count");
                return;
            }

            for(int i = 0 ; i < count ; ++i)
            {
                ++user_count;
                string user_name = "user-" + user_count.ToString();
                CUser user = new CUser(user_name, this);
                dic_user.Add(user_name, user);

                string[] arr = {user_name, GetStateString(user.State)};
                ListViewItem lv = new ListViewItem(arr);
                lv.Name = user_name;
                lv.BackColor = GetStateColor(user.State);

                listView_User.Items.Add(lv);
            }
        }

        // connect
        private void button1_Click(object sender, EventArgs e)
        {
            foreach(CUser user in dic_user.Values)
            {
                if (user.State != CUser.ConnState.None)
                    continue;

                user.State = CUser.ConnState.Connecting;
                user.Connect("127.0.0.1", 9000);
            }
        }

        // disconnect
        private void button2_Click(object sender, EventArgs e)
        {
            foreach (CUser user in dic_user.Values)
            {
                if (user.State != CUser.ConnState.Connected)
                    continue;

                user.Disconnect();
            }
        }


        // 채팅창 입력
        private void textBox1_PreviewKeyDown(object sender, PreviewKeyDownEventArgs e)
        {
            if(e.KeyCode == Keys.Enter)
            {
                if (current_user == null)
                {
                    Write("user is not selected");
                    return;
                }
                    

                if (current_user.State != CUser.ConnState.Connected)
                {
                    Write("user is not connected");
                    return;
                }

                CPacketStream stream = new CPacketStream();
                stream.SetString(textBox1.Text);

                current_user.Send(0x0100, stream.GetBuffer(), stream.GetSize());

                textBox1.Text = "";
                
            }
        }

        // 리스튜 뷰에서 유저 선택
        private void listView_User_SelectedIndexChanged(object sender, EventArgs e)
        {
            ListView.SelectedListViewItemCollection collection = listView_User.SelectedItems;
            if (collection.Count != 1)
                return;

            string username = collection[0].SubItems[0].Text;

            if (dic_user.ContainsKey(username) == false)
                return;

            CUser user = dic_user[username];

            if (current_user != user)
            {
                current_user = user;

                // 현재 유저의 채팅창 기준으로 표시
                listBox_Dlg.Items.Clear();
                foreach (string str in current_user.dig_list)
                {
                    listBox_Dlg.Items.Add(str);
                }
            }
        }

        // 채팅 메시지를 받았을 때, 현재 선택한 유저이면 갱신
        public void RenewalMsg(CUser user, string str)
        {
            if (current_user != user)
                return;

            if(listBox_Dlg.InvokeRequired)
            {
                Invoke(new MethodInvoker(delegate()
                {
                    if (listBox_Dlg.Items.Count > 100)
                        listBox_Dlg.Items.RemoveAt(0);

                    listBox_Dlg.Items.Add(str);

                }));
            }
            else
            {
                if (listBox_Dlg.Items.Count > 100)
                    listBox_Dlg.Items.RemoveAt(0);

                listBox_Dlg.Items.Add(str);
            }
        }

    }
}
