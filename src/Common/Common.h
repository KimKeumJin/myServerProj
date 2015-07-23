#pragma once

// Packet Header
#define PACKET_HEADER_SIZE 4

#pragma pack(push, 1)

struct PacketHeader
{
	unsigned short m_dataSize;
	unsigned short m_pid;
};

#pragma pack(pop)


// Packet ID
enum PID : unsigned short
{
	// reserved pid
	PID_NET_CONNECT,
	PID_NET_DISCONNECT,

	// user set pid
	PID_CHAT_MESSAGE	= 0x0100,
	PID_ROOMLIST,
	PID_CREATE_ROOM,
	PID_JOIN_ROOM,
	PID_LEAVE_ROOM,
};

