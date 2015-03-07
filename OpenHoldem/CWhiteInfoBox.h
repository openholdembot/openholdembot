//******************************************************************************
//
// This file is part of the OpenHoldem project
//   Download page:         http://code.google.com/p/openholdembot/
//   Forums:                http://www.maxinmontreal.com/forums/index.php
//   Licensed under GPL v3: http://www.gnu.org/licenses/gpl.html
//
//******************************************************************************
//
// Purpose:
//
//******************************************************************************

#ifndef INC_CWHITEINFOBOX_H
#define INC_CWHITEINFOBOX_H

class CWhiteInfoBox {
 public:
  CWhiteInfoBox();
  ~CWhiteInfoBox();
 public:
  void Draw(RECT client_rect, LOGFONT logfont, CDC *pDC,
    CPen *black_pen, CBrush *white_brush);
  // For log$-symbols
  void SetCustomLogMessage(CString message) { _custom_log_message = message; }
 private:
  CString InfoText();
 private:
  CString _custom_log_message;
};

#endif // INC_CWHITEINFOBOX_H