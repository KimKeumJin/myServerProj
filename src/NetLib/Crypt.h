#pragma once

class CCrypt
{
public:
	static BOOL Encrypt(BYTE *source, BYTE *destination, DWORD length);
	static BOOL Decrypt(BYTE *source, BYTE *destination, DWORD length);
};



// <��ȣȭ 1>
// RSA Ű ���� ����
// ���� �Ϸ�� Ŭ���̾�Ʈ�� �ۺ�Ű�� ������.(AcceptEx ������ ���� �޵��� �Ѵ�!!)
// ������ �ۺ� Ű�� �޴´�.
// �������ʹ� ������ �ۺ� Ű�� ��ȣȭ �Ͽ� ����
// ������ ���� ���� �����ʹ� Ŭ���� �����̺�Ű�� ��ȭȭ �Ͽ� �ص�
// (��ȣȭ Ű�� Ű���̺� �õ�� ����ص� �ȴ�.)

// <��ȣȭ 2>
// SEED�� ���� Encode, Decode ���̺��� ����(random Ŭ���� �ʿ�)
// Ư�� ���ؿ� �ٶ� Encode ���̺��� �����ͷ� xor ����

// Checksum ���
// PacketSendCount�� Ȱ���� ���⵵ ����
// ���� ID�� Ȱ���� ���⵵ ����