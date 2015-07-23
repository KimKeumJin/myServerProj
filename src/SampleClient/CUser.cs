using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SampleClient
{
    public class CUser : CNetwork
    {
        public enum ConnState
        {
            None,
            Connecting,
            Connected,
        };

        public string user_name;
        public List<string> dig_list = new List<string>();
        private SampleClientForm form;
        private ConnState con_state = ConnState.None;
        public ConnState State
        {
            get
            {
                return con_state;
            }
            set
            {
                con_state = value;
                form.SetUserState(this);
            }
        }

        public CUser(string user_name, SampleClientForm form)
        {
            this.user_name = user_name;
            this.form = form;
        }

        public override void Release()
        {
            base.Release();

            State = ConnState.None;
        }

        public override void Protocol(int packetID, CPacketStream packetStream)
        {
            switch(packetID)
            {
                case 256:   // chat message temp packet id
                    {
                        int serial = packetStream.GetI32();
                        string msg = packetStream.GetString();

                        if (dig_list.Count > 100)
                            dig_list.RemoveAt(0);

                        string str = string.Format("{0} : {1}", serial, msg);

                        dig_list.Add(str);

                        form.RenewalMsg(this, str);
                    }
                    break;

                default:
                    {
                        Write("protocol(invalid packet id)");
                    }
                    break;
            }
        }

        public override void Write(string msg)
        {
            form.Write(string.Format("{0} : {1}", user_name, msg));
        }
    }
}
