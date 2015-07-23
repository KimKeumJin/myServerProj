using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace SampleClient
{
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public class CPacketHeader
    {
        public int size;
        public int pid;
    }

    public class CPacketStream
    {
        const int MAX_BUFFER_SIZE = 4096;

        private byte[] buffer = null;
        private UInt16 offset = 0;

        public CPacketStream(int bufferSize = MAX_BUFFER_SIZE)
        {
            buffer = new byte[bufferSize];
        }

        public CPacketStream(byte[] _buffer)
        {
            int size = _buffer.Length;
            if (size > MAX_BUFFER_SIZE)
                size = MAX_BUFFER_SIZE;

            buffer = new byte[size];
            Buffer.BlockCopy(_buffer, 0, buffer, 0, size);
        }

        public CPacketStream(byte[] _buffer, int offset, int size)
        {
            if (size > MAX_BUFFER_SIZE)
                size = MAX_BUFFER_SIZE;

            buffer = new byte[size];
            Buffer.BlockCopy(_buffer, offset, buffer, 0, size);
        }

        //---------------------------------------------------
        public byte[] GetBuffer() { return buffer; }
        public UInt16 GetSize() { return offset; }

        //---------------------------------------------------
        public byte GetByte()
        {
            byte value = buffer[offset];
            offset += sizeof(byte);

            return value;
        }

        public Int16 GetI16()
        {
            Int16 value = BitConverter.ToInt16(buffer, offset);
            offset += sizeof(Int16);

            return value;
        }


        public UInt16 GetU16()
        {
            UInt16 value = BitConverter.ToUInt16(buffer, offset);
            offset += sizeof(UInt16);

            return value;
        }

        public Int32 GetI32()
        {
            Int32 value = BitConverter.ToInt32(buffer, offset);
            offset += sizeof(Int32);

            return value;
        }

        public UInt32 GetU32()
        {
            UInt32 value = BitConverter.ToUInt32(buffer, offset);
            offset += sizeof(UInt32);

            return value;
        }

        public Int64 GetI64()
        {
            Int64 value = BitConverter.ToInt64(buffer, offset);
            offset += sizeof(Int64);

            return value;
        }

        public UInt64 GetU64()
        {
            UInt64 value = BitConverter.ToUInt64(buffer, offset);
            offset += sizeof(UInt64);

            return value;
        }

        public string GetString()
        {
            // 글자 byte 수
            UInt16 length = GetU16();

            Encoding encoder = Encoding.GetEncoding(949);
            string value = encoder.GetString(buffer, offset, length);

            offset += (UInt16)(length + 1);

            return value;

        }

        public string GetWString()
        {
            // 글자수
            UInt16 length = GetU16();

            // 바이트수로 변환
            length = (UInt16)((length - 1) * 2);
            string value = ASCIIEncoding.Unicode.GetString(buffer, offset, length);

            offset += (UInt16)(length + 2);

            return value;

        }

        //---------------------------------------------------
        public bool SetI16(Int16 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(Int16));

            offset += sizeof(Int16);

            return true;
        }

        public bool SetU16(UInt16 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(UInt16));

            offset += sizeof(UInt16);

            return true;
        }

        public bool SetI32(Int32 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(Int32));

            offset += sizeof(Int32);

            return true;
        }

        public bool SetU32(UInt32 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(UInt32));

            offset += sizeof(UInt32);

            return true;
        }

        public bool SetI64(Int64 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(Int64));

            offset += sizeof(Int64);

            return true;
        }

        public bool SetU64(UInt64 value)
        {
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, buffer, offset, sizeof(UInt64));

            offset += sizeof(UInt64);

            return true;
        }

        // MBCS(Multi Byte Character Set)
        public bool SetString(string value)
        {
            Encoding encoder = Encoding.GetEncoding(949);
            byte[] arr = encoder.GetBytes(value);

            // 문자열 byte 수 저장
            if (SetU16((UInt16)(arr.Length + 1)) == false)
                return false;
            
            // 문자열 데이터 저장
            Buffer.BlockCopy(arr, 0, buffer, offset, arr.Length);
            offset += (UInt16)(arr.Length + 1);

            return true;
        }

        // UniCode
        public bool SetWString(string value)
        {

            byte[] arr = ASCIIEncoding.Unicode.GetBytes(value);

            SetI16((Int16)(value.Length + 1));

            Buffer.BlockCopy(arr, 0, buffer, offset, arr.Length);
            offset += (UInt16)(arr.Length + 2);

            return true;
        }
    }
}
