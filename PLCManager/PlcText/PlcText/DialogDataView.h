#pragma once


// CDialogDataView dialog

class CDialogDataView : public CDialog
{
	DECLARE_DYNAMIC(CDialogDataView)

public:
	CDialogDataView(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDialogDataView();

// Dialog Data
	enum { IDD = IDD_DIALOG_DATAVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void UpdateBlockData(byte *pBuffer,int nLength)
	{
		struct bytebit
		{
			byte bit0:1;
			byte bit1:1;
			byte bit2:1;
			byte bit3:1;
			byte bit4:1;
			byte bit5:1;
			byte bit6:1;
			byte bit7:1;
		};

		struct wordbit
		{
			byte bit0:1;
			byte bit1:1;
			byte bit2:1;
			byte bit3:1;
			byte bit4:1;
			byte bit5:1;
			byte bit6:1;
			byte bit7:1;
			byte bit8:1;
		};

		union UBytebit
		{
			byte	nByte;
			bytebit nBit;
		};
		if (IsDlgButtonChecked(IDC_CHECK_ENABLE) == BST_CHECKED)
		{
			//int nBlockID = GetDlgItemInt(IDC_EDIT_BLOCKID);
			int nBlockID = GetDlgItemInt(IDC_COMBO_BLOCKID);

			int nOffset = GetDlgItemInt(IDC_EDIT_OFFSET);
			if (nBlockID == 0 || nOffset >= nLength)
				return;
			int nDataType = -1;
			HWND hRadio = GetDlgItem(IDC_RADIO_BYTE)->GetSafeHwnd();
			for (int iButton = 0;iButton < 3;iButton ++)
			{
				if (::SendMessage(hRadio,BM_GETCHECK, 0, 0L) != 0)
					nDataType = iButton;
				hRadio = ::GetWindow(hRadio, GW_HWNDNEXT);
			}
			bool bBit = IsDlgButtonChecked(IDC_CHECK_BYTEBIT) == BST_CHECKED;
			TCHAR szValue[256];
			switch(nDataType)
			{
			case  -1:
			default:
				break;
			case 0:
				{
					UBytebit  uByte;
					uByte.nByte = pBuffer[nOffset];
					_stprintf(szValue,_T("%d(%d%d%d%d %d%d%d%d)"),uByte.nByte,
							uByte.nBit.bit7,
							uByte.nBit.bit6,
							uByte.nBit.bit5,
							uByte.nBit.bit4,
							uByte.nBit.bit3,
							uByte.nBit.bit2,
							uByte.nBit.bit1,
							uByte.nBit.bit0);
					SetDlgItemText(IDC_EDIT_DATA,szValue);

				}
				break;
			case 1:
				break;
			case 2:
				break;
			}
		}
	}

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckEnable();
	int m_nDataType;
	int m_nDataType1;
};
