using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace SampleClient
{
    // 네트워크 접속, 패킷 전달 등을 처리하는 클래스
    public abstract class CNetwork
    {
        private const int RECV_BUFFER_LENGTH = 4096;

        private Socket m_Sock = null;
        private byte[] m_recvBuffer = new byte[RECV_BUFFER_LENGTH];
        private byte[] m_saveBuffer = new byte[RECV_BUFFER_LENGTH * 3];
        private int m_offset = 0;
        private bool m_isShutdown = false;

        public abstract void Protocol(int packetID, CPacketStream packetStream);
        public abstract void Write(string msg);
        public virtual void Release()
        {
            m_offset = 0;
        }

        public bool Connect(string ip, int port)
        {
            try
            {
                m_Sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                IPEndPoint iep = new IPEndPoint(IPAddress.Parse(ip), port);


                m_Sock.BeginConnect(iep, new AsyncCallback(ConnectCallBack), this);

                return true;
            }
            catch (System.Exception ex)
            {
                Release();

                Write(ex.ToString());

                return false;
            }
        }

        public bool Connect(UInt32 ip, int port)
        {
            try
            {
                m_Sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);

                IPEndPoint iep = new IPEndPoint(ip, port);

                m_Sock.BeginConnect(iep, new AsyncCallback(ConnectCallBack), this);

                return true;
            }
            catch (System.Exception ex)
            {
                Release();

                Write(ex.ToString());

                return false;
            }
        }

        public void Disconnect()
        {
            if (m_isShutdown)
                return;

            m_isShutdown = true;

            if (m_Sock.Connected)
                m_Sock.Shutdown(SocketShutdown.Both);

            //m_Sock.Close();

            // 설정 초기화
            Release();

        }

        private void ConnectCallBack(IAsyncResult ar)
        {
            CUser user = ar.AsyncState as CUser;

            try
            {
                m_Sock.EndConnect(ar);

                m_isShutdown = false;

                BeginAsyncReceive();

                user.State = CUser.ConnState.Connected;

            }
            catch (System.Exception ex)
            {
                Release();

                Write(ex.ToString());
            }
        }

        private void BeginAsyncReceive()
        {
            try
            {
                m_Sock.BeginReceive(m_recvBuffer, 0, m_recvBuffer.Length,
                    SocketFlags.None, new AsyncCallback(ReceiveCallBack), this);
            }
            catch (System.Exception ex)
            {
                Release();

                Write(ex.ToString());
            }
        }

        private void ReceiveCallBack(IAsyncResult ar)
        {
            try
            {
                int recvBytes = m_Sock.EndReceive(ar);
                if (recvBytes == 0)
                {
                    Disconnect();
                    return;
                }

                Buffer.BlockCopy(m_recvBuffer, 0, m_saveBuffer, m_offset, recvBytes);
                m_offset += recvBytes;


                // 패킷 파싱
                // 최소한 헤더사이즈만큼 받았는지 검사
                while (m_offset >= SampleConstant.PACKET_HEADER_SIZE)
                {
                    CPacketHeader header = new CPacketHeader();
                    header.size = BitConverter.ToUInt16(m_saveBuffer, 0);
                    header.pid = BitConverter.ToUInt16(m_saveBuffer, 2);

                    // 패킷 최대 크기 검사
                    int packetSize = header.size + SampleConstant.PACKET_HEADER_SIZE;
                    if (packetSize > 4096)
                    {
                        Disconnect();
                        Write("Invalid Packet Size");
                        return;
                    }

                    // 패킷데이터 크기 검사
                    if (m_offset < packetSize)
                        break;

                    // 정상적인 패킷
                    CPacketStream packetStream = new CPacketStream(m_saveBuffer, SampleConstant.PACKET_HEADER_SIZE, packetSize - SampleConstant.PACKET_HEADER_SIZE);

                    // 버퍼 리셋
                    Buffer.BlockCopy(m_saveBuffer, packetSize, m_saveBuffer, 0, m_offset - packetSize);
                    m_offset -= packetSize;

                    if (m_offset < 0)
                    {
                        Disconnect();
                        Write("Invalid Offset Position");
                        return;
                    }

                    Protocol(header.pid, packetStream);
                }

                // 패킷 수신 대기 상태로 전환
                BeginAsyncReceive();
            }
            catch (System.ObjectDisposedException ex)
            {
                Release();

                Write(ex.ToString());
            }
            catch (System.Exception ex)
            {
                Disconnect();

                Write(ex.ToString());
            }
        }

        public void Send(UInt16 packetID, byte[] data, UInt16 length)
        {
            try
            {
                int packetLength = SampleConstant.PACKET_HEADER_SIZE + length;
                byte[] sendData = new byte[packetLength];


                // 패킷 구성
                Buffer.BlockCopy(BitConverter.GetBytes(length), 0, sendData, 0, 2);
                Buffer.BlockCopy(BitConverter.GetBytes(packetID), 0, sendData, 2, 2);
                Buffer.BlockCopy(data, 0, sendData, SampleConstant.PACKET_HEADER_SIZE, length);


                m_Sock.BeginSend(sendData, 0, sendData.Length, SocketFlags.None, new AsyncCallback(SendCallback), m_Sock);
            }
            catch (System.Exception ex)
            {
                Disconnect();

                Write(ex.ToString());
            }
        }

        private void SendCallback(IAsyncResult ar)
        {
            Socket sock = ar.AsyncState as Socket;

            try
            {
                int sendBytes = sock.EndSend(ar);
            }
            catch (System.Exception ex)
            {
                Disconnect();

                Write(ex.ToString());
            }
        }
    }
}
