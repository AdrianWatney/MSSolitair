
// ChildView.cpp : implementation of the CChildView class for solitair
// Solitair 
// there are 7 stacks of cards to turn over by removing the last card not turned over.
//  The layoutt is as follows
//		[A]	[A]	[A]	[A]				[CS]				[A]=ace stacks, [CS]=Closed stack
//	[S]	[S]	[S]	[S]	[S]	[S]	[S]		[OS]				[S]=7 stacks,	[OS]=open stacks	
// 
// Rules:
// The stacks are dealt from left to right leaving the last card open which gives the number of cards on each stack as : 1,2,3,4,5,6,7 The remaining cards 24 are placed on the closed stack
// The cards on the stack must be in decending order and in the oppisite suit color
// The cards in the closed stack can be opened up one at a time until the last is opened then all cards are copyed back to the closed stack and you start again, all cards can be taken from the  open stack until no more cards are left.
// At the beginning there is one turned over card on each 7 stack You can place opened cards on these open cards in decending order. A 7 stack with no more cards on is open, a king can be placed onto this stack.
// The aces can be placed into the 4 ace stacks and the stacks grown with the next card  of the same suit until the king  when all ace stacks are full the game is wone.
// If there are no more closed cards in the 7 stacks, the game can be wone and is effectivly finnished.
// 
// Functionality:
// The left mouse button is used to play the game you can click on the closed pack and a card will be transfered from the top of the closed stack to the top of the open stack and shown, when there are no more cards in the closed pack 
// picking it will cause the open pack to be turned over and placed back on the closed pack. 
// The mouse can also pick open cards from the 7 stacks and drag them to anouther 7 stacks individualy or as a stack.
// The aces can be placed in a free aces stack from any open stack. The kings can be placed into any open 7 stack from any open stack. all other cards can be placed onto a stack (not the open stack) that has a matching corisponding card. 
// A single click on a selected open top card will cause a search of the Aces stacks first and then the 7 stacks next to find a position automaticaly if found will move card to the stack.
// A single click and hold of a bunch of cards will allow you the drags the cards and place the draged pile onto anouther stack if the card order matchs
// To force the cards to a specific stack you must press and hold the left mouse button and drag the cards to the stack you want to move them to. 
// 					
#include "pch.h"
#include "framework.h"
#include "Solitair.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment (lib,"Gdiplus.lib")

// CChildView
#define AFONTSIZE		48

CString Suits[] = {_T("H"),_T("D"),_T("S"),_T("C")};	// Hearts, Diamonds, Spades, Clubs

CChildView::CChildView()
{
	int i;
	for (i = 0; i < 52; i++)
		m_cardpack[i] = i + 1;
	memset(&m_selectedcards, 0, sizeof(m_selectedcards));
	m_numberofselectedcards = 0;
	m_bleftbutton = FALSE;
	m_bFinnish = FALSE;
	m_highscore = 0;
	m_timerstarted = FALSE;
	m_newhighscore = FALSE;
	m_gamefinnished = FALSE;
	gdiplusToken = 0;
	memset(m_cardresources,0,sizeof(m_cardresources));
	memset(&m_undorec, 0, sizeof(m_undorec));
	m_timetofinishgame = 0;
	m_startgametimer = 0;
	m_cardmargin = 0;
	m_numberclosedpack = 0;
	m_numberopenpack = 0;
	memset(m_numbercardaces, 0, sizeof(m_numbercardaces));
	memset(m_numbercard7stacks, 0, sizeof(m_numbercard7stacks));
	memset(m_openpack, 0, sizeof(m_openpack));
	memset(m_closedpack, 0, sizeof(m_closedpack));
	memset(m_cardaces, 0, sizeof(m_cardaces));
	memset(m_card7stacks, 0, sizeof(m_card7stacks));
	memset(m_shuffledcards, 0, sizeof(m_shuffledcards));
	m_cardback = NULL;
}

CChildView::~CChildView()
{
	GdiplusShutdown(gdiplusToken);
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_FILE_NEWGAME, &CChildView::OnFileNewgame)
	ON_COMMAND(ID_EDIT_UNDO, &CChildView::OnEditUndo)
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	time_t			t;
	WCHAR			path[256];
	DWORD			len, r;
	CString			dir;
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	GdiplusStartupInput gdiplusStartupInput;
	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	m_highscore = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HighScore"), -1);
	m_datadir = AfxGetApp()->GetProfileString(_T("Settings"), _T("CardResourcesDir"), _T(""));
	if (m_datadir.GetLength() < 2) {
		// card resource directory not yet defined
		len = 256;
		r = GetCurrentDirectory(len, path);
		dir = path;
		m_datadir.Format(_T("%s\\Cards"), dir);	// assume directory under current directory
		AfxGetApp()->WriteProfileString(_T("Settings"), _T("CardResourcesDir"), m_datadir);
	}
	time(&t);
	srand((unsigned int)t);	// seed random number generator
	LoadResources();
	ShuffelCards();
	SetupLayout();
	SaveUndoRecord();
	return TRUE;
}

void CChildView::OnPaint() 
{
	int				col,cnum,hh, ww, mm, x, y;
	int				sh, cw,ch,cwh,chh;
	double			ra;
	CString			s;
	RECT			r,cr;
	time_t			t;
	SolidBrush		solidBrush(Color(255, 0, 0, 0));
	SolidBrush		bkb(Color(255,255, 255, 255));
	//FontFamily		fontFamily(L"Arial");
	FontFamily		fontFamily(L"Times New Roman");
	Gdiplus::Font   font(&fontFamily, AFONTSIZE, FontStyleRegular, UnitPoint);
	RectF			rr;
	PointF			ptf;
	CBrush			bsh(RGB(0,150,0));
	SolidBrush		bsh1(Color(255, 0, 150, 0));
	CTimeSpan		tspan;
		
	CPaintDC dc(this); // device context for painting
	
	// the screen size canges so first calculate layout for screen
	GetClientRect(&r);
	Pen pna(Color(0, 0, 255), 1);	// pen color for margin on aces stacks
	Pen pns(Color(0, 255, 0), 2);	// selected pen
	ww = r.right - r.left;			// screen width
	hh = r.bottom - r.top;			// screen height
	cw = ww / 9;					// calculate the card width bassed on 9 cards across the screen
	mm = cw / 4;					// the margin is a 1/4 of a card
	ch = (cw * 4) / 3;				// card height is 4/3 of card width
	sh = (mm * 16) + (ch * 2);		// screen height needed for max screen width to show all cards.
	if (sh > hh) {		// screen height is not enough start with using screen height to calculate card size
		ra = hh / ww;	// screen ratio
		ch = hh / 5;	// card height equals the screen height / 5
		cw = (ch * 3) / 4;	// card width equals 3/4 of the card height
		mm = cw / 4;		// margin is 1/4 of card width
	}
	m_cardmargin = mm;
	Bitmap bmp(ww, hh);		// create a bitmap for the whole screen
	Gdiplus::Graphics graphics(&bmp);	// create a graphics for the bitmap
	if (m_bFinnish) {
		// we are in the finnish stage find anouther card to finnish, return rectangle of card moving
		if (!MoveCardToFinnish(&cr)) {
			m_bFinnish = FALSE;	// finished drawing the finish
		}
		// fill areas changing
		dc.FillRect(&cr, &bsh);			// fill screen with green background
		graphics.FillRectangle(&bsh1, cr.left, cr.top, cr.right-cr.left, cr.bottom-cr.top);				// fill screen with green background
	} else 
		graphics.FillRectangle(&bsh1, 0, 0, ww, hh);				// fill screen with green background

	cwh = (cw / 2)- (int)(font.GetSize()/1.5); chh = (ch/ 2) - (int)font.GetSize(); 
	cr.left = 0; cr.top = 0; cr.right = cw; cr.bottom = ch;		// card rectangle
	x = (mm*2) + cw; y = mm;	// x,y = starting position on screen top,left
	// ------------ Aces -----------------
	// we have 4 packs on top for Aces and to the right the closed pack of cards
	s = _T("A");	// write an A on each aces stack
	
	ptf.X = (REAL)x + cwh; ptf.Y = (REAL)y + chh;
	for (col = 0; col < 4; col++)	// there are 4 aces stacks
	{
		graphics.DrawRectangle(&pna, x, y, cw, ch);
		if (m_numbercardaces[col] == 0)		// pack empty
			graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush);	// "A"
		else
			graphics.DrawImage(m_cardaces[col][m_numbercardaces[col] - 1].img, x + 1, y + 1, cw - 2, ch - 2);	// draw card on aces stack
		m_cardaces[col][0].r.left = x; m_cardaces[col][0].r.right = x + cw;	// save position of aces pack
		m_cardaces[col][0].r.top = y; m_cardaces[col][0].r.bottom = y + ch;
		x += cw + mm;		// advance to next aces pack across
		ptf.X = (REAL)x + cwh; ptf.Y = (REAL)y + chh;	// update string of next aces stack
	}
	// ------------ Closed pack -----------------
	// draw the closed pack
	x += (cw*3) + (mm*4);		// calculate x position of closed stack
	if (m_numberclosedpack > 0)
		graphics.DrawImage(m_cardback, x + 1, y + 1, cw - 2, ch - 2);	// still cards in closed pack to display, show card back.
	else
		graphics.FillRectangle(&bkb, x, y, cw - 2, ch - 2);				// no closed cards left show empty card
	graphics.DrawRectangle(&pna, x, y, cw, ch);	// draw outline of closed card stack
	m_closedpack[0].r.left = x; m_closedpack[0].r.right = x + cw;	// save position of closed pack
	m_closedpack[0].r.top = y; m_closedpack[0].r.bottom = y + ch;
	// ------------ Open pack -----------------
	// draw the open pack
	x -= mm; y += ch + mm;		// calculate x,y position of open stack
	
	if(m_numberopenpack>0) 
		graphics.DrawImage(m_openpack[m_numberopenpack-1].img, x + 1, y + 1, cw - 2, ch - 2);	// draw top card on open stack
	else
		graphics.FillRectangle(&bkb, x, y, cw - 2, ch - 2);		// no open cards show empty card
	graphics.DrawRectangle(&pna, x, y, cw, ch);					// draw outline of open card stack
	m_openpack[0].r.left = x; m_openpack[0].r.right = x + cw;	// save position of open pack
	m_openpack[0].r.top = y; m_openpack[0].r.bottom = y + ch;
	// ------------ Stacks -----------------
	// under this we have 7 packs for displayed cards and to the right the opened pack of cards
	x = mm; y = ch + (mm*2);			// x,y = starting position on screen top,left
	for (col = 0; col < 7; col++) {		// for each column
		y = ch + (mm * 2);				// calculate top for card 7 stacks
		for (cnum = 0; cnum < m_numbercard7stacks[col]; cnum++) {	// for each card in column
			graphics.DrawRectangle(&pna, x, y, cw, ch);
			if (m_card7stacks[col][cnum].id.shown == 1)		// card shown
				graphics.DrawImage(m_card7stacks[col][cnum].img, x + 1, y + 1, cw - 2, ch - 2);	// draw shown card
			else 
				graphics.DrawImage(m_cardback, x + 1, y + 1, cw - 2, ch - 2);	// draw card back
			m_card7stacks[col][cnum].r.left = x; m_card7stacks[col][cnum].r.top = y; m_card7stacks[col][cnum].r.right = x + cw; m_card7stacks[col][cnum].r.bottom = y + ch;	// save position of card on 7 stack, needed for mouse point select
			y = ch + (mm * (cnum+3));	// calculate next card down stack top
		}
		if (m_numbercard7stacks[col] == 0) {		// no cards in this 7 stack
			graphics.FillRectangle(&bkb, x, y, cw - 2, ch - 2);
			graphics.DrawRectangle(&pna, x, y, cw, ch);
		}
		x += cw + mm;	// next colum of 7 stacks
	}
	
	if (m_numberofselectedcards > 0 && m_selectedcards[m_numberofselectedcards-1].r.right > 0) {
		// we have a selected card
		if (m_bleftbutton) {	// move selected card rectangle
			graphics.DrawRectangle(&pns, m_newpoint.x, m_newpoint.y, cw, ch);
		} else
			graphics.DrawRectangle(&pns, m_selectedcards[m_numberofselectedcards-1].r.left, m_selectedcards[m_numberofselectedcards-1].r.top, cw, ch);
	}
	if (GetCountOfUnshown7StackCards() == 0) {
		// game can be finnished
		if (m_gamefinnished && m_numbercardaces[0] == 13 && m_numbercardaces[1] == 13 && m_numbercardaces[2] == 13 && m_numbercardaces[3] == 13) {
			s.Format(_T("Game finnished you have won in %d:%d"), m_timetofinishgame / 60, m_timetofinishgame % 60);
			if (m_newhighscore) {
				// game finnished
				s.Format(_T("You have Won NEW HIGH score: %d:%d"), m_highscore / 60, m_highscore % 60);
			}
		}
		else
			s = "Game can be (f)innished";
		ptf.X = (REAL)ww / 2; ptf.Y = (REAL)hh/2;
		graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush);	// display can finnish game
		// if all aces stacks are full
		if (!m_gamefinnished && m_numbercardaces[0] == 13 && m_numbercardaces[1] == 13 && m_numbercardaces[2] == 13 && m_numbercardaces[3] == 13)
		{
			// finnish game.
			s.Format(_T("You have Won"));
			ptf.Y += font.GetSize();
			graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush); // dispaly game finnished
			if (m_timerstarted) {
				time(&t);
				m_timerstarted = FALSE;		// game finnished
				m_gamefinnished = TRUE;
				t -= m_startgametimer;
				m_timetofinishgame = (int)t;
				if (m_highscore > t || m_highscore<0) {
					m_highscore = (int)t;
					s.Format(_T("New High score: %d:%d"), t / 60, t % 60);
					AfxGetApp()->WriteProfileInt(_T("Settings"), _T("HighScore"), m_highscore);	// save high score
					m_newhighscore = TRUE;
				} else 
					s.Format(_T("In Time: %d:%d"), t / 60, t % 60);
				ptf.X = (REAL)mm; ptf.Y = (REAL)hh - (font.GetSize() * 2);
				graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush); // dispaly game time
			}
		}
	}
	// display timer
	if (m_timerstarted) {
		time(&t);
		t -= m_startgametimer;
		s.Format(_T("Time: %d:%d"), t/60, t%60);
		ptf.X = (REAL)mm; ptf.Y = (REAL)hh - (font.GetSize()*2);
		graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush); // dispaly game time
	}
	

	Graphics grph(dc.m_hDC);
	grph.DrawImage(&bmp, 0, 0);
}

void CChildView::LoadResources()
{
	int		i,j;
	CString	s;
	// load card resources
	CStringW	wc;
	
	wc.Format(_T("%s\\CardBack.png"), m_datadir);
	m_cardback = Image::FromFile(wc);

	memset(m_cardresources, 0, sizeof(m_cardresources));
	for (i = 0;i < 4; i++) {
		for (j = 1; j < 14; j++) {
			wc.Format(_T("%s\\%d%s.png"), m_datadir,j, Suits[i]);
			m_cardresources[(i*13)+j].img = Image::FromFile(wc);
			m_cardresources[(i * 13) + j].id.suit = i;
			m_cardresources[(i * 13) + j].id.cardnumber = j;
		}
	}
	// setup game cards stacks.
	memset(m_card7stacks, 0, sizeof(m_card7stacks));			//	m_card7stacks[7][18]	
	memset(m_cardaces, 0, sizeof(m_cardaces));				//	m_cardaces[4][13]
	memset(m_closedpack, 0, sizeof(m_closedpack));			//	m_closedpack[24]
	memset(m_openpack, 0, sizeof(m_openpack));				//	m_openpack[24]
	memset(m_numbercard7stacks, 0, sizeof(m_numbercard7stacks));	//	m_numbercard7stacks[7]	number of cards in stack
	memset(m_numbercardaces, 0, sizeof(m_numbercardaces));		//	m_numbercardaces[4]		number of cards in stack
	m_numberclosedpack = 0;									//	number of cards in stack
	m_numberopenpack = 0;									//  number of cards in open stack
}

void CChildView::SaveUndoRecord()
{
	memcpy(m_undorec.m_card7stacks, m_card7stacks, sizeof(m_card7stacks));		//	m_card7stacks[7][18]	
	memcpy(m_undorec.m_cardaces, m_cardaces, sizeof(m_cardaces));				//	m_cardaces[4][13]
	memcpy(m_undorec.m_closedpack, m_closedpack, sizeof(m_closedpack));			//	m_closedpack[24]
	memcpy(m_undorec.m_openpack, m_openpack, sizeof(m_openpack));				//	m_openpack[24]
	memcpy(m_undorec.m_numbercard7stacks, m_numbercard7stacks, sizeof(m_numbercard7stacks));	//	m_numbercard7stacks[7]	number of cards in stack
	memcpy(m_undorec.m_numbercardaces, m_numbercardaces, sizeof(m_numbercardaces));		//	m_numbercardaces[4]		number of cards in stack
	m_undorec.m_numberclosedpack = m_numberclosedpack;									//	number of cards in stack
	m_undorec.m_numberopenpack = m_numberopenpack;										//  number of cards in open stack
}

void CChildView::LoadUndoRecord()
{
	memcpy(m_card7stacks, m_undorec.m_card7stacks,  sizeof(m_card7stacks));		//	m_card7stacks[7][18]	
	memcpy(m_cardaces, m_undorec.m_cardaces,  sizeof(m_cardaces));				//	m_cardaces[4][13]
	memcpy(m_closedpack, m_undorec.m_closedpack,  sizeof(m_closedpack));		//	m_closedpack[24]
	memcpy(m_openpack, m_undorec.m_openpack,  sizeof(m_openpack));				//	m_openpack[24]
	memcpy(m_numbercard7stacks, m_undorec.m_numbercard7stacks,  sizeof(m_numbercard7stacks));	//	m_numbercard7stacks[7]	number of cards in stack
	memcpy(m_numbercardaces, m_undorec.m_numbercardaces,  sizeof(m_numbercardaces));	//	m_numbercardaces[4]		number of cards in stack
	m_numberclosedpack = m_undorec.m_numberclosedpack;									//	number of cards in stack
	m_numberopenpack=m_undorec.m_numberopenpack;										//  number of cards in open stack
}

void CChildView::ShuffelCards()
{
	int		i,c;

	memcpy(m_shuffledcards, m_cardpack, sizeof(m_cardpack));
	for (i = 51; i > 0; i--) {
		int j = rand() % (i + 1); 
		// swap byte from array[i] with array[j]
		c = m_shuffledcards[i];
		m_shuffledcards[i] = m_shuffledcards[j];
		m_shuffledcards[j] = c;
	}
}

void CChildView::SetupLayout()
{
	int		i,cnt;
	cnt = 0;		// current card from shuffled pack
	// deal the cards in the same order as a dealer
	for (i = 0; i < 7; i++) {
		m_card7stacks[i][0] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 1; i < 7; i++) {
		m_card7stacks[i][1] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 2; i < 7; i++) {
		m_card7stacks[i][2] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 3; i < 7; i++) {
		m_card7stacks[i][3] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 4; i < 7; i++) {
		m_card7stacks[i][4] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 5; i < 7; i++) {
		m_card7stacks[i][5] = m_cardresources[m_shuffledcards[cnt++]];
	}
	for (i = 6; i < 7; i++) {
		m_card7stacks[i][6] = m_cardresources[m_shuffledcards[cnt++]];
	}
	m_card7stacks[0][0].id.shown = 1;	// only show the lasst card
	m_card7stacks[1][1].id.shown = 1;
	m_card7stacks[2][2].id.shown = 1;
	m_card7stacks[3][3].id.shown = 1;
	m_card7stacks[4][4].id.shown = 1;
	m_card7stacks[5][5].id.shown = 1;
	m_card7stacks[6][6].id.shown = 1;

	for (i = 0; i < 7; i++) 
		m_numbercard7stacks[i] = i+1;	// number of cards in each stack
	for (i = 0; i < 4; i++)
		m_numbercardaces[i] = 0;		// number of cards in aces suit packs
	m_numberclosedpack = 0;
	while (cnt < 52) {
		m_closedpack[m_numberclosedpack++] = m_cardresources[m_shuffledcards[cnt++]];	// fill up closed pack
	}
	m_numberopenpack = 0;
}

void CChildView::OnLButtonDown(UINT /* nFlags */, CPoint point)
{
	m_dragpoint = point;		// starting point to drag from
	m_numberofselectedcards = 0;
	if (GetSelectedCardsUnderPt(point))	// we need to get selection here for dragging
	{
		m_bleftbutton = TRUE;
		//InvalidateRect(NULL, TRUE);
	}
	if (!m_timerstarted && !m_gamefinnished) {
		// start game timer
		time(&m_startgametimer);
		m_timerstarted = TRUE;
	}
}

void CChildView::OnLButtonUp(UINT /* nFlags */, CPoint point)
{
	CRect			rc;

	if (OpenStackOneMore(point)) {			// if point over closed stack and no closed cards left turn cards back from open cards stack
		m_numberofselectedcards = 0;
		m_bleftbutton = FALSE;
		rc = m_openpack[0].r;
		InvalidateRect(&rc, FALSE);
	}
	if (m_bleftbutton)		// left button down prepared
	{
		m_bleftbutton = FALSE;
		if (m_numberofselectedcards > 0)	// cards selected
		{
			if (!MoveSelectedCardsToPt(point))		// try to move selected cards to stack under point 
				MoveSelectedCard();			//  not moved, try to move selected card to a maching stack 
			InvalidateRect(NULL, FALSE);
		}
	}
	m_numberofselectedcards = 0;	// unselect card
}

void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	int		dx, dy, cw,ch;
	CRect	r;
	if (m_bleftbutton) {	// move selected card
		if (m_numberofselectedcards > 0)
		{
			cw = m_selectedcards[m_numberofselectedcards-1].r.right - m_selectedcards[m_numberofselectedcards-1].r.left;
			ch = m_selectedcards[m_numberofselectedcards-1].r.bottom - m_selectedcards[m_numberofselectedcards-1].r.top;
			dx = m_dragpoint.x - point.x;
			dy = m_dragpoint.y - point.y;
			m_newpoint.x = m_selectedcards[m_numberofselectedcards-1].r.left - dx;
			m_newpoint.y = m_selectedcards[m_numberofselectedcards-1].r.top - dy;
			r.left = m_newpoint.x; r.top = m_newpoint.y;
			r.right = m_newpoint.x + cw; r.bottom = m_newpoint.y + ch;
			InvalidateRect(&r, FALSE);
		}
	}
}

CARDRESOURES CChildView::GetCard7StackUnderPt(CPoint point)
{
	CARDRESOURES	card;
	int				i,j;
	CRect			rc;
	memset(&card, 0, sizeof(card));
	// you can only pick the last card in each stack
	for (i = 0; i < 7; i++) {						// for all columns
		for (j = 0; j < m_numbercard7stacks[i]; j++) {	// for each card in column
			rc = m_card7stacks[i][j].r;
			if (rc.PtInRect(point)) {
				if (m_card7stacks[i][j].id.shown) {	// if more than one card shown select top 
					card = m_card7stacks[i][j];
				}
			}
		}
	}
	return card;
}

CARDRESOURES CChildView::GetOpenCardUnderPt(CPoint point)
{
	CARDRESOURES	card;
	CRect			rc;
	memset(&card, 0, sizeof(card));
	// you can only pick the last card in each stack
	if (m_numberopenpack) {		// closed pack has cards to open
		rc = m_openpack[0].r;
		if (rc.PtInRect(point)) {
			card = m_openpack[m_numberopenpack - 1]; // this card has been double clicked
			card.r = m_openpack[0].r;	// copy rectangle from first card as all cards are on top of each other
		}
	}
	return card;
}

CARDRESOURES CChildView::GetAcesCardUnderPt(CPoint point)
{
	int				i;
	CARDRESOURES	card;
	CRect			rc;
	memset(&card, 0, sizeof(card));
	for (i = 0; i < 4; i++) {						// for all columns
		rc = m_cardaces[i][0].r;
		if (rc.PtInRect(point)) {
			if (m_numbercardaces[i] > 0) {
				card = m_cardaces[i][m_numbercardaces[i] - 1]; // 
				card.r = m_cardaces[i][0].r;	// copy rectangle from first card as all cards are on top of each other
			}
		}
	}
	return card;
}
int CChildView::GetAcesStackUnderPt(CPoint point)
{
	int				i;
	CRect			rc;
	
	for (i = 0; i < 4; i++) {
		rc = m_cardaces[i][0].r;
		if (rc.PtInRect(point)) {
			return i;
		}
	}
	return -1;
}
int CChildView::Get7StackUnderPt(CPoint point)
{
	int				i;
	CRect			rc;
	for (i = 0; i < 7; i++) {						// for all stack columns
		rc = m_card7stacks[i][0].r;
		rc.bottom += m_cardmargin * m_numbercard7stacks[i];
		if (rc.PtInRect(point)) {
			return i;
		}
	}
	return -1;
}
int CChildView::Get7StackWithImage(CARDRESOURES card)
{
	int				i,j;
	CRect			rc;
	
	for (i = 0; i < 7; i++) {
		for (j = 0; j < m_numbercard7stacks[i]; j++) {
			if (m_card7stacks[i][j].img == card.img)
			{
				return i;
			}
		}
	}
	return -1;
}

int CChildView::GetAceStackWithImage(CARDRESOURES card)
{	// only check top aces card 
	int				i;
	
	for (i = 0; i < 4; i++) {
		if (m_numbercardaces[i]>0 && m_cardaces[i][m_numbercardaces[i]-1].img == card.img)
		{
			return i;
		}
	}
	return -1;
}

BOOL CChildView::GetOpenStackWithImage(CARDRESOURES card)
{	// only check top aces card 
	if (m_numberopenpack == 0) return FALSE;
	return m_openpack[m_numberopenpack-1].img==card.img;
}

int CChildView::GetCountOfUnshown7StackCards()
{
	int		i,j,cnt = 0;
	for (i = 0; i < 7; i++) {
		for (j = 0; j < m_numbercard7stacks[i]; j++) {
			if (m_card7stacks[i][j].id.shown == 0) cnt++;
		}
	}
	return cnt;
}

BOOL CChildView::MoveSelectedCard()
{	// if this is a drag and drop then place cards where they where dropped, else find matching card
	int			i,j,si;
	CARDRESOURES	card,topselectedcard;
	topselectedcard = m_selectedcards[0];
	si = Get7StackWithImage(topselectedcard);
	if (si >= 0) {	
		// selected card on a 7 stack
		// first check if selected card is an ace
		if (topselectedcard.id.cardnumber == 1 && m_numberofselectedcards == 1) {
			// selected card is an ace must can go on free aces stack
			for (i = 0; i < 4; i++) {
				if (m_numbercardaces[i] == 0) {
					SaveUndoRecord();
					// this ace stack free for an ace
					m_cardaces[i][0] = topselectedcard;
					m_numbercardaces[i]++;
					// remove selected card from stack
					m_numbercard7stacks[si]--;
					if (m_numbercard7stacks[si] > 0)
						m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
					m_numberofselectedcards = 0;
					return TRUE;
				}
			}
		}
		else {
			// not an ace, next check if card can go on an aces pack
			if (m_numberofselectedcards == 1) {
				for (i = 0; i < 4; i++) {
					if (m_numbercardaces[i] > 0) {
						card = m_cardaces[i][m_numbercardaces[i] - 1];	// current card on aces stack
						if (card.id.suit == topselectedcard.id.suit) {	// card of the same suit
							if (card.id.cardnumber + 1 == topselectedcard.id.cardnumber) {	// card can go onto this stack
								SaveUndoRecord();
								m_cardaces[i][m_numbercardaces[i]] = topselectedcard;
								m_numbercardaces[i]++;
								// remove selected card from stack
								m_numbercard7stacks[si]--;
								if (m_numbercard7stacks[si] > 0)
									m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
								m_numberofselectedcards = 0;
								return TRUE;
							}
						}
					}
				}
			}
			// search stacks to find if top card can be move to new position.
			for (i = 0; i < 7; i++) {
				if (i != si) {
					// selected card not on this stack
					if (m_numbercard7stacks[i] > 0) {
						card = m_card7stacks[i][m_numbercard7stacks[i] - 1];
						if ((topselectedcard.id.suit < 2 && card.id.suit >= 2) || (topselectedcard.id.suit >= 2 && card.id.suit < 2))
						{	// cards of different color
							if ((card.id.cardnumber - topselectedcard.id.cardnumber) == 1)
							{	// next card number down.
								SaveUndoRecord();
								// we can move cards here
								// copy all selected cards from select stack
								for (j = 0; j < m_numberofselectedcards; j++)
								{
									m_selectedcards[j].id.shown = 1;
									m_card7stacks[i][m_numbercard7stacks[i]++] = m_selectedcards[j];
								}
								// remove all selected cards from 7 stack
								m_numbercard7stacks[si]-= m_numberofselectedcards;
								if (m_numbercard7stacks[si] > 0)
									m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
							
								m_numberofselectedcards = 0;
								return TRUE;
							}
						}
					}
					else {
						// this 7 stack is free see if selected card is a king
						if (topselectedcard.id.cardnumber == 13) {
							// its a king can move it here
							SaveUndoRecord();
							// copy all selected cards from select stack
							for (j = 0; j < m_numberofselectedcards; j++)
							{
								m_selectedcards[j].id.shown = 1;
								m_card7stacks[i][m_numbercard7stacks[i]++] = m_selectedcards[j];
							}
							// remove all selected cards from 7 stack
							m_numbercard7stacks[si] -= m_numberofselectedcards;
							if (m_numbercard7stacks[si] > 0)
								m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;

							m_numberofselectedcards = 0;
							return TRUE;
						}
					}
				}
			}
		}
	}
	else {	// not on 7 stack
		if (GetOpenStackWithImage(topselectedcard)) {
			// selected card on open stack
			// first check if selected card is an ace
			if (topselectedcard.id.cardnumber == 1) {
				for (i = 0; i < 4; i++) {
					if (m_numbercardaces[i] == 0) {
						SaveUndoRecord();
						// this ace stack free for an ace
						m_cardaces[i][0] = topselectedcard;
						m_numbercardaces[i]++;
						// remove selected card from open stack
						m_numberopenpack--;
						m_numberofselectedcards = 0;
						return TRUE;
					}
				}
			}
			else {	// selected card not an ace
				// search aces stacks first
				for (i = 0; i < 4; i++) {
					if (topselectedcard.id.cardnumber == m_cardaces[i][m_numbercardaces[i] - 1].id.cardnumber + 1 && topselectedcard.id.suit == m_cardaces[i][m_numbercardaces[i] - 1].id.suit) {
						SaveUndoRecord();
						m_cardaces[i][m_numbercardaces[i]++] = topselectedcard;		// move to aces stack
						// remove selected card from open stack
						m_numberopenpack--;
						m_numberofselectedcards = 0;
						return TRUE;
					}
				}
				// next search 7 stacks
				for (i = 0; i < 7; i++) {
					if (m_numbercard7stacks[i] > 0) {
						if (topselectedcard.id.cardnumber == m_card7stacks[i][m_numbercard7stacks[i] - 1].id.cardnumber - 1) {	// selected card is the next card down on this stack
							if ((topselectedcard.id.suit < 2 && m_card7stacks[i][m_numbercard7stacks[i] - 1].id.suit >= 2) || (topselectedcard.id.suit >= 2 && m_card7stacks[i][m_numbercard7stacks[i] - 1].id.suit < 2))
							{	// cards of different color can move card here
								SaveUndoRecord();
								topselectedcard.id.shown = 1;	// card is shown.
								m_card7stacks[i][m_numbercard7stacks[i]] = topselectedcard;
								m_numbercard7stacks[i]++;
								// remove selected card from open stack
								m_numberopenpack--;
								m_numberofselectedcards = 0;
								return TRUE;
							}
						}
					}
					else {
						// can move king to empty 7 stack
						if (topselectedcard.id.cardnumber == 13) {	// selected card is a king and this is an empty 7 stack
							SaveUndoRecord();
							topselectedcard.id.shown = 1;	// card is shown.
							m_card7stacks[i][0] = topselectedcard;
							m_numbercard7stacks[i]++;
							// remove selected card from open stack
							m_numberopenpack--;
							m_numberofselectedcards = 0;
							return TRUE;
						}
					}
				}
			}
		}
	}
	return FALSE;
}

BOOL CChildView::MoveSelectedCardsToPt(CPoint topoint)
{
	int				i,si,stack7,stackaces;
	CARDRESOURES	topselectedcard,topcardtostack;
	if (m_numberofselectedcards > 0)
	{
		topselectedcard = m_selectedcards[0];
		stack7 = Get7StackUnderPt(topoint);
		if (stack7 >= 0) 
		{	// to point over 7 stack
			si = Get7StackWithImage(topselectedcard);
			if (si == stack7) {
				// same stack check for straight move
				if(MoveSelectedCard())
					return TRUE;
			}
			if (m_numbercard7stacks[stack7] == 0) 
			{		// empty 7 stack
				if (topselectedcard.id.cardnumber == 13)
				{	// only kings can go onto an empty 7 stack
					si = Get7StackWithImage(topselectedcard);
					SaveUndoRecord();
					// copy all selected cards from select stack
					for (i = 0; i < m_numberofselectedcards; i++)
					{
						m_selectedcards[i].id.shown = 1;
						m_card7stacks[stack7][i] = m_selectedcards[i];
						m_numbercard7stacks[stack7]++;
					}
					// remove cards from the from stack
					if (si >= 0)
					{	// selected cards came fron 7 stacks
						m_numbercard7stacks[si] -= m_numberofselectedcards;
						m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
						m_numberofselectedcards = 0;	// unselect card
						return TRUE;
					}
					else {
						for (i = 0; i < m_numberopenpack; i++)
						{		// open pack has cards open
							if (m_openpack[i].img == topselectedcard.img) {
								// this is the top selected card in the open stack
								m_numberopenpack -= m_numberofselectedcards;
								m_numberofselectedcards = 0;	// unselect card
								return TRUE;
							}
						}
						// check if card from aces pack
						si = GetAceStackWithImage(topselectedcard);
						if (si >= 0) {
							// selected card on aces pack must be top card
							m_numbercardaces[si]--;
							m_numberofselectedcards = 0;	// unselect card
							return TRUE;
						}
					}
				}
			} else {	// to  7 stack not empty
				topcardtostack = m_card7stacks[stack7][m_numbercard7stacks[stack7] - 1];
				if ((topcardtostack.id.cardnumber - topselectedcard.id.cardnumber) == 1)
				{	// next card number down.
					// {_T("H"),_T("D"),_T("S"),_T("C")}
					if ((topselectedcard.id.suit < 2 && topcardtostack.id.suit >= 2) || (topselectedcard.id.suit >= 2 && topcardtostack.id.suit < 2))
					{	// cards of different color
						si = Get7StackWithImage(topselectedcard);
						SaveUndoRecord();
						// move card to new stack
						for (i = 0; i < m_numberofselectedcards; i++)
						{
							m_selectedcards[i].id.shown = 1;
							m_card7stacks[stack7][m_numbercard7stacks[stack7]] = m_selectedcards[i];
							m_numbercard7stacks[stack7]++;
						}
						// remove cards from the from stack
						if (si >= 0) 
						{	// selected cards came fron 7 stacks
							m_numbercard7stacks[si] -= m_numberofselectedcards;
							m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
							m_numberofselectedcards = 0;	// unselect card
							return TRUE;
						} else {
							for(i=0;i< m_numberopenpack;i++)
							{		// open pack has cards open
								if (m_openpack[i].img == topselectedcard.img) {
									// this is the top selected card in the open stack
									m_numberopenpack -= m_numberofselectedcards;
									m_numberofselectedcards = 0;	// unselect card
									return TRUE;
								}
							}
							// check if card from aces pack
							si = GetAceStackWithImage(topselectedcard);
							if (si >= 0) {
								// selected card on aces pack must be top card
								m_numbercardaces[si]--;
								m_numberofselectedcards = 0;	// unselect card
								return TRUE;
							}
						}						
					}
				}
			}
		}
		stackaces = GetAcesStackUnderPt(topoint);
		if (m_numberofselectedcards == 1 && stackaces >= 0) {	// only one card at a time on aces to point over aces stack
			if (m_numbercardaces[stackaces] == 0) 
			{		// empty ace stack
				if (topselectedcard.id.cardnumber == 1) 
				{	// only Aces can go onto an empty ace stack
					si = Get7StackWithImage(topselectedcard);
					SaveUndoRecord();
					topselectedcard.id.shown = 1;
					m_cardaces[stackaces][m_numbercardaces[stackaces]] = topselectedcard;
					m_numbercardaces[stackaces]++;
					// remove card from the from stack
					if (si >= 0) {	// selected cards came fron 7 stacks
						m_numbercard7stacks[si]--;
						m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
						m_numberofselectedcards = 0;	// unselect card
						return TRUE;
					}
					else {
						for (i = 0; i < m_numberopenpack; i++)
						{		// open pack has cards open
							if (m_openpack[i].img == topselectedcard.img) {
								// this is the top selected card in the open stack
								m_numberopenpack -= m_numberofselectedcards;
								m_numberofselectedcards = 0;	// unselect card
								return TRUE;
							}
						}
					}
				}
			} else {	// to aces stack not empty
				topcardtostack = m_cardaces[stackaces][m_numbercardaces[stackaces] - 1];
				if ((topselectedcard.id.cardnumber - topcardtostack.id.cardnumber) == 1)
				{	// next card number up.
					if (topselectedcard.id.suit == topcardtostack.id.suit)
					{	// cards of same suit
						si = Get7StackWithImage(topselectedcard);
						SaveUndoRecord();
						// move card to new stack
						topselectedcard.id.shown = 1;
						m_cardaces[stackaces][m_numbercardaces[stackaces]] = topselectedcard;
						m_numbercardaces[stackaces]++;
						// remove card from the from stack
						if (si >= 0) {	// selected cards came fron 7 stacks
							m_numbercard7stacks[si]--;
							m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
							m_numberofselectedcards = 0;	// unselect card
							return TRUE;
						}
						else {
							for (i = 0; i < m_numberopenpack; i++)
							{		// open pack has cards open
								if (m_openpack[i].img == topselectedcard.img) {
									// this is the top selected card in the open stack
									m_numberopenpack--;
									m_numberofselectedcards = 0;	// unselect card
									return TRUE;
								}
							}
						}
					}
				}
			}
		}
	}
	return FALSE;
}
BOOL	CChildView::GetSelectedCardsUnderPt(CPoint point)
{
	int				i, j, k;
	CARDRESOURES	card;

	m_numberofselectedcards = 0;
	card = GetCard7StackUnderPt(point);	// get card under point from 7 stacks.
	if (card.id.cardnumber > 0) {
		// card selected from 7 stacks
		m_selectedcards[m_numberofselectedcards++] = card;	// move card to first selected
		// if card selected is not last in stack then find all following cards and select them all
		j = Get7StackUnderPt(point);
		if (m_numbercard7stacks[j] > 1) {
			// if the card is not the top one on the stack copy all cards on the stack to selected
			if (m_card7stacks[j][0].img == card.img) {
				// all stack selected add any more cards
				for (k = 1; k < m_numbercard7stacks[j]; k++)
					m_selectedcards[m_numberofselectedcards++] = m_card7stacks[j][k];

				return TRUE;

			}
			else {	// not all of stack selected
				for (i = m_numbercard7stacks[j] - 1; i > 0; i--) {
					if (m_card7stacks[j][i].img == card.img) {
						m_numberofselectedcards = 0;	// start selection from selected card and add next cards to selection
						for (k = i; k < m_numbercard7stacks[j]; k++)
							m_selectedcards[m_numberofselectedcards++] = m_card7stacks[j][k];
						return TRUE;
					}
				}
			}
		}
		return TRUE;
	}
	card = GetOpenCardUnderPt(point);	// get card under point for open stack
	if (card.id.cardnumber > 0) {
		// card selected from open stack
		card.r = m_openpack[0].r;	// needs rectangle to drag
		card.id.shown = 1;
		m_selectedcards[m_numberofselectedcards++] = card;
		return TRUE;
	}
	card = GetAcesCardUnderPt(point);	// get card under point for aces stacks
	if (card.id.cardnumber > 0) {
		// card selected from aces stacks
		card.id.shown = 1;
		m_selectedcards[m_numberofselectedcards++] = card;
		return TRUE;
	}
	return FALSE;
}

BOOL	CChildView::OpenStackOneMore(CPoint point)
{
	int				i, j;
	CRect			rc;
	// if over closed pack open card
	rc = m_closedpack[0].r;				// are we over the closed pack
	if (rc.PtInRect(point)) {
		if (m_numberclosedpack==0) {
			SaveUndoRecord();
			// no cards in closed pack to turn over so put opened cards back
			j = 0;
			for (i = m_numberopenpack; i > 0; i--) {
				m_closedpack[j++] = m_openpack[i - 1];
			}
			m_closedpack[0].r = rc;		// put back the rectangle to first card on stack
			m_numberclosedpack = m_numberopenpack;
			m_numberopenpack = 0;
			InvalidateRect(&m_openpack[0].r, TRUE);
			InvalidateRect(&m_closedpack[0].r, TRUE);
			return TRUE;			
		}
		else {
			// turn next card in closed pack to open pack
			SaveUndoRecord();
			rc = m_openpack[0].r;	// save open pack rectangle
			m_openpack[m_numberopenpack++] = m_closedpack[m_numberclosedpack - 1];
			m_numberclosedpack--;
			m_openpack[0].r = rc;	// if this is first card opened then replace its closed rectangle with the open rectangle 
			InvalidateRect(&m_openpack[0].r, TRUE);
			InvalidateRect(&m_closedpack[0].r, TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

void CChildView::OnFileNewgame()
{
	// user has requested a new game
	m_bFinnish = FALSE;
	m_timerstarted = FALSE;
	m_newhighscore = FALSE;
	m_gamefinnished = FALSE;
	ShuffelCards();
	SetupLayout();
	InvalidateRect(NULL, TRUE);
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'f' || nChar <= 'F') {
		// Finnish the solitair game
		if (GetCountOfUnshown7StackCards() == 0) {
			// (f)innish
			m_bFinnish = TRUE;			// start finnishing game
			SetTimer('NR', 400, 0);		// animate card move	
		}
	}
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	CString	s;
	RECT	r;
	if (nIDEvent == 'NR')
	{	// timer to animate finnish cards by forceing redraw
		GetClientRect(&r);
		InvalidateRect(&r, FALSE);
		if (!m_bFinnish) {
			KillTimer(nIDEvent);
		}
	}
}

BOOL CChildView::MoveCardToFinnish(RECT	*r)
{
	int				i,ii,j;
	CARDRESOURES	card,c;
	if (m_bFinnish)
	{
		// find card to move to aces stack
		for (i = 0; i < 4; i++)
		{
			if (m_numbercardaces[i] > 0) {
				// this ace stack has cards
				card = m_cardaces[i][m_numbercardaces[i]-1];	// top card on aces stack
				for (j = 0; j < 7; j++) {	// find a matching card on the 7 stack
					if (m_numbercard7stacks[j]) {
						c = m_card7stacks[j][m_numbercard7stacks[j]-1];	// top card
						if (card.id.suit == c.id.suit) {
							// 7 stack card suit matches aces suit
							if (card.id.cardnumber == c.id.cardnumber - 1) {
								// card can be moved
								m_cardaces[i][m_numbercardaces[i]++] = c;	// move card to aces stack
								m_numbercard7stacks[j]--;					// remove card from 7 stack
								// return rectangle of moved card
								*r = m_card7stacks[j][m_numbercard7stacks[j]].r;
								return TRUE;
							}
						}
					}
				}
				// search the open stack 
				for (j = 0; j < m_numberopenpack; j++) {
					c = m_openpack[j];
					if (card.id.suit == c.id.suit) {
						// open stack card suit matches aces suit
						if (card.id.cardnumber == c.id.cardnumber - 1) {
							// card can be moved
							m_cardaces[i][m_numbercardaces[i]++] = c;	// move card to aces stack
							for (ii = j+1; ii < m_numberopenpack; ii++) {	// remove card from open stack
								c = m_openpack[ii];		// card after card going to aces stack
								m_openpack[ii-1]=c;		// shuffle down
							}
							m_numberopenpack--;
							// return rectangle of moved card
							*r = m_openpack[0].r;
							return TRUE;
						}
					}
				}
				// search the closed pack 
				for (j = 0; j < m_numberclosedpack; j++) {
					c = m_closedpack[j];
					if (card.id.suit == c.id.suit) {
						// closed stack card suit matches aces suit
						if (card.id.cardnumber == c.id.cardnumber - 1) {
							// card can be moved
							m_cardaces[i][m_numbercardaces[i]++] = c;	// move card to aces stack
							for (ii = j + 1; ii < m_numberclosedpack; ii++) {	// remove card from open stack
								c = m_closedpack[ii];		// card after card going to aces stack
								m_closedpack[ii - 1] = c;		// shuffle down
							}
							m_numberclosedpack--;
							// return rectangle of moved card
							*r = m_closedpack[0].r;
							return TRUE;
						}
					}
				}
			}
			else {
				// we need to search the closed and open cards for an ace
				for (ii = 0; ii < m_numberclosedpack; ii++) {
					if (m_closedpack[ii].id.cardnumber == 1) {
						// found ace in closed pack at i
						for (j = ii+1; j < m_numberclosedpack; j++) {
							c = m_closedpack[j];
							m_openpack[m_numberopenpack++] = c;
						}
						m_cardaces[i][m_numbercardaces[i]++] = m_closedpack[ii];	// move card to aces stack
						m_numberclosedpack = ii;
						// return rectangle of moved card
						*r = m_openpack[0].r;
						return TRUE;
					}
				}
			}
		}	// for i=0;i<4
	}
	memset(r, 0, sizeof(RECT));
	return FALSE;
}


void CChildView::OnEditUndo()
{
	// player wants to undo last move.
	LoadUndoRecord();
	InvalidateRect(NULL, TRUE);
}
