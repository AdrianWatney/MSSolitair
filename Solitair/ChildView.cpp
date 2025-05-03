
// ChildView.cpp : implementation of the CChildView class for solitair
// This file contains the implementation of the CChildView class, which handles the Solitaire game logic, rendering, and user interactions.
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
#include "CBestTimesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma comment (lib,"Gdiplus.lib")

// CChildView
#define AFONTSIZE		48

// Array of card suits used in the game: Hearts, Diamonds, Spades, Clubs
CString Suits[] = {_T("H"),_T("D"),_T("S"),_T("C")};	// Hearts, Diamonds, Spades, Clubs

// Constructor: Initializes the game state and member variables.
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
    memset(m_besttimes, 0, sizeof(m_besttimes));
    m_newscoreindex = 0;
    m_cardmargin = 0;                  // Margin between cards, calculated during drawing  
    m_cardheight = 0;				  // card height in screen units
    m_cardwidth = 0;				  // card width in screen units
    m_cardfont=NULL;		  // card aces stack font for empty pack
    m_cardrectpen=NULL;				  // card pen to draw rectangle with
    m_tablecolorbrush=NULL;	  // the card table brush
    m_screenheight=0;
}

// Destructor: Cleans up resources used by the game.
CChildView::~CChildView()
{
	GdiplusShutdown(gdiplusToken);
}

// Message map for handling Windows messages.
BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_FILE_NEWGAME, &CChildView::OnFileNewgame)
	ON_COMMAND(ID_EDIT_UNDO, &CChildView::OnEditUndo)
    ON_COMMAND(ID_HELP_BESTTIMES, &CChildView::OnHelpBesttimes)
END_MESSAGE_MAP()

// PreCreateWindow: Configures the window before it is created.
// Initializes GDI+, sets up the window style, and loads game resources.
BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
   time_t t;
   WCHAR path[256];
   DWORD len, r;
   CString dir;

   if (!CWnd::PreCreateWindow(cs))
       return FALSE;

   GdiplusStartupInput gdiplusStartupInput;
   // Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

   cs.dwExStyle |= WS_EX_CLIENTEDGE;
   cs.style &= ~WS_BORDER;
   cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
       ::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), nullptr);

   // Load high score and card resource directory from application settings.
   GetBestTimes();
   //m_highscore = AfxGetApp()->GetProfileInt(_T("Settings"), _T("HighScore"), -1);
   m_datadir = AfxGetApp()->GetProfileString(_T("Settings"), _T("CardResourcesDir"), _T(""));
   if (m_datadir.GetLength() < 2) {
       // Card resource directory not yet defined.
       len = 256;
       r = GetCurrentDirectory(len, path);
       dir = path;
       m_datadir.Format(_T("%s\\Cards"), dir); // Assume directory under current directory.
       AfxGetApp()->WriteProfileString(_T("Settings"), _T("CardResourcesDir"), m_datadir);
   }

   // Seed the random number generator for shuffling cards.
   time(&t);
   srand((unsigned int)t);

   // Load resources, shuffle cards, and set up the initial game layout.
   LoadResources();
   ShuffelCards();
   SetupLayout();
   SaveUndoRecord();

   return TRUE;
}

// OnPaint: Handles the WM_PAINT message to render the game.
// Draws the game layout, cards, and other UI elements.
void CChildView::OnPaint() 
{
	int				hh, ww, mm;
	int				sh, cw,ch;
	double			ra;
	CString			s;
	RECT			r,cr;
	
	SolidBrush		solidBrush(Color(255, 0, 0, 0));
	SolidBrush		bkb(Color(255,255, 255, 255));
	//FontFamily		fontFamily(L"Arial");
	FontFamily		fontFamily(L"Times New Roman");
	Gdiplus::Font   font(&fontFamily, AFONTSIZE, FontStyleRegular, UnitPoint);
	RectF			rr;
	PointF			ptf;
	CBrush			bsh(RGB(0,150,0));
	SolidBrush		bsh1(Color(255, 0, 150, 0));
		
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
    // these varables are available during OnPaint
	m_cardmargin = mm;
    m_cardheight = ch;
    m_cardwidth = cw;
    m_cardfont = &font;
    m_cardrectpen = &pna;
    m_tablecolorbrush = &bsh1;
    m_screenheight = hh;
	Bitmap bmp(ww, hh);		// create a bitmap for the whole screen
	Gdiplus::Graphics graphics(&bmp);	// create a graphics for the bitmap
    // ----------- finnished -------------------
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

	
	// ------------ Aces -----------------
	// we have 4 packs on top for Aces and to the right the closed pack of cards
    DrawAcesPacks(&graphics);
	
	// ------------ Closed pack -----------------
	// draw the closed pack
    DrawClosedPack(&graphics);
    
	// ------------ Open pack -----------------
	// draw the open pack
    DrawOpenPack(&graphics);

	// ------------ Stacks -----------------
	// under this we have 7 packs for displayed cards and to the right the opened pack of cards
    DrawStacksPacks(&graphics);
	
	if (m_numberofselectedcards > 0 && m_selectedcards[m_numberofselectedcards-1].r.right > 0) {
		// we have a selected card
		if (m_bleftbutton) {	// move selected card rectangle
			graphics.DrawRectangle(&pns, m_newpoint.x, m_newpoint.y, cw, ch);
		} else
			graphics.DrawRectangle(&pns, m_selectedcards[m_numberofselectedcards-1].r.left, m_selectedcards[m_numberofselectedcards-1].r.top, cw, ch);
	}
	if (GetCountOfUnshown7StackCards() == 0) {
		// game can be finnished
		if (m_gamefinnished) {
			s.Format(_T("Game finnished you have won in %d:%02d"), m_timetofinishgame / 60, m_timetofinishgame % 60);
			if (m_newhighscore) {
				// game finnished
				s.Format(_T("You have Won NEW HIGH score[%d]: %d:%02d"), m_newscoreindex+1, m_highscore / 60, m_highscore % 60);
                if (m_newscoreindex == 0) {
                    // new highest score
                    s.Format(_T("New Best ever High score: %d:%02d"), m_highscore / 60, m_highscore % 60);
                }
			}
            ptf.X = (REAL)ww / 6; ptf.Y = (REAL)hh / 2;     // center, left
		}
        else {
            s = "Game can be (f)innished";
            ptf.X = (REAL)ww / 2; ptf.Y = (REAL)hh - font.GetSize() * 2;    // bottom, center
        }
        graphics.FillRectangle(&bsh1, ptf.X, ptf.Y, font.GetSize()* s.GetLength(), font.GetSize() * 2);    // clear text background
		graphics.DrawString(s, s.GetLength(), &font, ptf, &solidBrush);	// display can finnish game

		// if all aces stacks are full
		if (!m_gamefinnished && m_numbercardaces[0] == 13 && m_numbercardaces[1] == 13 && m_numbercardaces[2] == 13 && m_numbercardaces[3] == 13)
		{
			// finnish game.
            FinnishGame();
		}
	}
	// display timer at bottom of screen
    DisplayTimer(&graphics);

	Graphics grph(dc.m_hDC);
	grph.DrawImage(&bmp, 0, 0);
}

// LoadResources: Loads card images and initializes game resources.
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

// SaveUndoRecord: Saves the current game state for undo functionality.
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

// LoadUndoRecord: Restores the game state from the last saved undo record.
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

// ShuffelCards: Shuffles the deck of cards for a new game.
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

// SetupLayout: Sets up the initial layout of cards for a new game. Deals cards
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
	m_card7stacks[0][0].id.shown = 1;	// only show the last card on each stack
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
   m_dragpoint = point; // Store the starting point of the drag operation.  
   m_numberofselectedcards = 0; // Reset the number of selected cards.  

   // Check if there are cards under the point to select for dragging.  
   if (GetSelectedCardsUnderPt(point))  
   {  
       m_bleftbutton = TRUE; // Indicate that the left mouse button is pressed.  
       // InvalidateRect(NULL, TRUE); // Uncomment if you want to force a redraw.  
   }  

   // Start the game timer if it hasn't started and the game isn't finished.  
   if (!m_timerstarted && !m_gamefinnished)  
   {  
       time(&m_startgametimer); // Record the start time of the game.  
       m_timerstarted = TRUE; // Mark the timer as started.  
   }  
}  

// OnLButtonUp: Handles the left mouse button release event.  
// This function performs the following actions:  
// - Checks if the user clicked on the closed stack to open one more card.  
// - If the left mouse button was pressed and cards were selected, it attempts to move the selected cards.  
// - If the selected cards cannot be moved to a valid stack, it tries to move them to a matching stack.  
// - Resets the state of selected cards and redraws the affected areas.  
void CChildView::OnLButtonUp(UINT /* nFlags */, CPoint point)  
{  
  CRect rc;  

  // Check if the point is over the closed stack to open one more card.  
  if (OpenStackOneMore(point))  
  {  
      m_numberofselectedcards = 0; // Reset the number of selected cards.  
      m_bleftbutton = FALSE; // Reset the left button state.  
      rc = m_openpack[0].r; // Get the rectangle of the open pack.  
      InvalidateRect(&rc, FALSE); // Redraw the open pack area.  
  }  

  // If the left button was pressed, handle the release logic.  
  if (m_bleftbutton)  
  {  
      m_bleftbutton = FALSE; // Reset the left button state.  

      // If there are selected cards, try to move them.  
      if (m_numberofselectedcards > 0)  
      {  
          // Attempt to move the selected cards to the stack under the point.  
          if (!MoveSelectedCardsToPt(point))  
              MoveSelectedCard(); // If not moved, try to move the selected card to a matching stack.  

          InvalidateRect(NULL, FALSE); // Redraw the entire window.  
      }  
  }  

  m_numberofselectedcards = 0; // Reset the number of selected cards.  
}

// OnMouseMove: Handles the mouse move events for dragging selected cards.  
// This function calculates the drag offset and updates the position of the dragged card(s).  
// It also invalidates the area of the dragged card to trigger a redraw.  
//  
// Parameters:  
// - nFlags: Indicates whether various virtual keys are down.  
// - point: A CPoint object representing the current mouse cursor position.  
void CChildView::OnMouseMove(UINT nFlags, CPoint point)  
{  
  int dx, dy, cw, ch;  
  CRect r;  

  // If the left button is pressed, handle dragging of the selected card(s).  
  if (m_bleftbutton)  
  {  
      if (m_numberofselectedcards > 0)  
      {  
          // Calculate the width and height of the selected card.  
          cw = m_selectedcards[m_numberofselectedcards - 1].r.right - m_selectedcards[m_numberofselectedcards - 1].r.left;  
          ch = m_selectedcards[m_numberofselectedcards - 1].r.bottom - m_selectedcards[m_numberofselectedcards - 1].r.top;  

          // Calculate the drag offset.  
          dx = m_dragpoint.x - point.x;  
          dy = m_dragpoint.y - point.y;  

          // Update the new position of the dragged card.  
          m_newpoint.x = m_selectedcards[m_numberofselectedcards - 1].r.left - dx;  
          m_newpoint.y = m_selectedcards[m_numberofselectedcards - 1].r.top - dy;  

          // Define the rectangle for the dragged card.  
          r.left = m_newpoint.x;  
          r.top = m_newpoint.y;  
          r.right = m_newpoint.x + cw;  
          r.bottom = m_newpoint.y + ch;  

          InvalidateRect(&r, FALSE); // Redraw the dragged card area.  
      }  
  }  
}

// GetCard7StackUnderPt: Finds the card in the 7-stack under a given point.  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
// Returns:  
// - A CARDRESOURES object representing the card found under the point.  
//   If no card is found, the returned CARDRESOURES object will be initialized to zero.  
CARDRESOURES CChildView::GetCard7StackUnderPt(CPoint point)  
{  
  CARDRESOURES card;  
  int i, j;  
  CRect rc;  

  memset(&card, 0, sizeof(card)); // Initialize the card structure to zero.  

  // Iterate through all columns of the 7 stacks.  
  for (i = 0; i < 7; i++)  
  {  
      // Iterate through each card in the column.  
      for (j = 0; j < m_numbercard7stacks[i]; j++)  
      {  
          rc = m_card7stacks[i][j].r; // Get the rectangle of the card.  

          // Check if the point is within the card's rectangle.  
          if (rc.PtInRect(point))  
          {  
              // If the card is shown, select the top card.  
              if (m_card7stacks[i][j].id.shown)  
              {  
                  card = m_card7stacks[i][j];  
              }  
          }  
      }  
  }  

  return card; // Return the card found under the point.  
}

// GetOpenCardUnderPt: Finds the top card in the open stack under a given point.  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
// Returns:  
// - A CARDRESOURES object representing the card found under the point.  
//   If no card is found, the returned CARDRESOURES object will be initialized to zero.  
CARDRESOURES CChildView::GetOpenCardUnderPt(CPoint point)  
{  
  CARDRESOURES card;  
  CRect rc;  

  memset(&card, 0, sizeof(card)); // Initialize the card structure to zero.  

  // Check if there are cards in the open stack.  
  if (m_numberopenpack)  
  {  
      rc = m_openpack[0].r; // Get the rectangle of the open pack.  

      // Check if the point is within the open pack's rectangle.  
      if (rc.PtInRect(point))  
      {  
          card = m_openpack[m_numberopenpack - 1]; // Get the top card of the open stack.  
          card.r = m_openpack[0].r; // Copy the rectangle from the first card.  
      }  
  }  

  return card; // Return the card found under the point.  
}

// GetAcesCardUnderPt: Finds the top card in the Ace stack under a given point.  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
// Returns:  
// - A CARDRESOURES object representing the card found under the point.  
//   If no card is found, the returned CARDRESOURES object will be initialized to zero.  
CARDRESOURES CChildView::GetAcesCardUnderPt(CPoint point)  
{  
  int i;  
  CARDRESOURES card;  
  CRect rc;  

  memset(&card, 0, sizeof(card)); // Initialize the card structure to zero.  

  // Iterate through all columns of the Aces stacks.  
  for (i = 0; i < 4; i++)  
  {  
      rc = m_cardaces[i][0].r; // Get the rectangle of the Aces stack.  

      // Check if the point is within the Aces stack's rectangle.  
      if (rc.PtInRect(point))  
      {  
          // If the stack has cards, get the top card.  
          if (m_numbercardaces[i] > 0)  
          {  
              card = m_cardaces[i][m_numbercardaces[i] - 1];  
              card.r = m_cardaces[i][0].r; // Copy the rectangle from the first card.  
          }  
      }  
  }  

  return card; // Return the card found under the point.  
}

// GetAcesStackUnderPt: Determines which Ace stack is under a given point.  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
// Returns:  
// - The index of the Ace stack (0-3) if the point is within an Ace stack's rectangle.  
// - -1 if no Ace stack is found under the point.  
int CChildView::GetAcesStackUnderPt(CPoint point)  
{  
  int i;  
  CRect rc;  

  // Iterate through all Aces stacks.  
  for (i = 0; i < 4; i++)  
  {  
      rc = m_cardaces[i][0].r; // Get the rectangle of the Aces stack.  

      // Check if the point is within the Aces stack's rectangle.  
      if (rc.PtInRect(point))  
      {  
          return i; // Return the index of the Aces stack.  
      }  
  }  

  return -1; // Return -1 if no stack is found under the point.  
}

// Get7StackUnderPt: Determines which 7-stack is under a given point.  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
// Returns:  
// - The index of the 7-stack (0-6) if the point is within a 7-stack's rectangle.  
// - -1 if no 7-stack is found under the point.  
int CChildView::Get7StackUnderPt(CPoint point)  
{  
  int i;  
  CRect rc;  

  // Iterate through all 7 stacks.  
  for (i = 0; i < 7; i++)  
  {  
      rc = m_card7stacks[i][0].r; // Get the rectangle of the 7 stack.  
      rc.bottom += m_cardmargin * m_numbercard7stacks[i]; // Adjust the bottom of the rectangle based on the number of cards.  

      // Check if the point is within the 7 stack's rectangle.  
      if (rc.PtInRect(point))  
      {  
          return i; // Return the index of the 7 stack.  
      }  
  }  

  return -1; // Return -1 if no stack is found under the point.  
}

// Get7StackWithImage: Finds the index of the 7-stack containing the specified card.  
// Parameters:  
// - card: The CARDRESOURES object representing the card to search for.  
// Returns:  
// - The index of the 7-stack (0-6) if the card is found.  
// - -1 if the card is not found in any 7-stack.  
int CChildView::Get7StackWithImage(CARDRESOURES card)  
{  
  int i, j;  

  // Iterate through all 7 stacks.  
  for (i = 0; i < 7; i++)  
  {  
      // Iterate through each card in the stack.  
      for (j = 0; j < m_numbercard7stacks[i]; j++)  
      {  
          // Check if the card's image matches the given card's image.  
          if (m_card7stacks[i][j].img == card.img)  
          {  
              return i; // Return the index of the stack.  
          }  
      }  
  }  

  return -1; // Return -1 if no stack is found with the card's image.  
}


// GetAceStackWithImage: Finds the index of the Ace stack containing the specified card.  
// Parameters:  
// - card: The CARDRESOURES object representing the card to search for.  
// Returns:  
// - The index of the Ace stack (0-3) if the card is found.  
// - -1 if the card is not found in any Ace stack.  
int CChildView::GetAceStackWithImage(CARDRESOURES card)  
{  
  int i;  

  // Iterate through all Aces stacks.  
  for (i = 0; i < 4; i++)  
  {  
      // Check if the top card of the stack matches the given card's image.  
      if (m_numbercardaces[i] > 0 && m_cardaces[i][m_numbercardaces[i] - 1].img == card.img)  
      {  
          return i; // Return the index of the stack.  
      }  
  }  

  return -1; // Return -1 if no stack is found with the card's image.  
}

// GetOpenStackWithImage: Checks if a specific card is on top of the open stack.  
// Parameters:  
// - card: A CARDRESOURES object representing the card to check.  
// Returns:  
// - TRUE if the card is on top of the open stack.  
// - FALSE if the open stack is empty or the card is not on top.  
BOOL CChildView::GetOpenStackWithImage(CARDRESOURES card)  
{  
  // Check if the open stack is empty.  
  if (m_numberopenpack == 0)  
      return FALSE;  

  // Check if the top card of the open stack matches the given card's image.  
  return m_openpack[m_numberopenpack - 1].img == card.img;  
}

// GetCountOfUnshown7StackCards: Counts the number of cards in the 7 stacks that are not shown.  
// This function iterates through all the cards in the 7 stacks and checks if they are not visible.  
// Returns:  
// - The total count of unshown cards in the 7 stacks.  
int CChildView::GetCountOfUnshown7StackCards()  
{  
  int i, j, cnt = 0;  

  // Iterate through all 7 stacks.  
  for (i = 0; i < 7; i++)  
  {  
      // Iterate through each card in the stack.  
      for (j = 0; j < m_numbercard7stacks[i]; j++)  
      {  
          // Count the cards that are not shown.  
          if (m_card7stacks[i][j].id.shown == 0)  
              cnt++;  
      }  
  }  

  return cnt; // Return the count of unshown cards.  
}

// MoveSelectedCard: Handles the logic for moving a selected card or group of cards.  
// This function determines if the selected card(s) can be moved to a valid position  
// based on the rules of Solitaire. It checks for valid moves to ace stacks,  
// 7-stacks, or open stacks. If a valid move is found, the card(s) are moved,  
// and the game state is updated.  
//  
// Returns:  
// - TRUE if the selected card(s) were successfully moved.  
// - FALSE if no valid move was found.  
BOOL CChildView::MoveSelectedCard()  
{  
   // if this is a drag and drop then place cards where they where dropped, else find matching card  
   int i, j, si;  
   CARDRESOURES card, topselectedcard;  
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
                       card = m_cardaces[i][m_numbercardaces[i] - 1]; // current card on aces stack  
                       if (card.id.suit == topselectedcard.id.suit) { // card of the same suit  
                           if (card.id.cardnumber + 1 == topselectedcard.id.cardnumber) { // card can go onto this stack  
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
                       { // cards of different color  
                           if ((card.id.cardnumber - topselectedcard.id.cardnumber) == 1)  
                           { // next card number down.  
                               SaveUndoRecord();  
                               // we can move cards here  
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
   else { // not on 7 stack  
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
           else { // selected card not an ace  
               // search aces stacks first  
               for (i = 0; i < 4; i++) {  
                   if (topselectedcard.id.cardnumber == m_cardaces[i][m_numbercardaces[i] - 1].id.cardnumber + 1 && topselectedcard.id.suit == m_cardaces[i][m_numbercardaces[i] - 1].id.suit) {  
                       SaveUndoRecord();  
                       m_cardaces[i][m_numbercardaces[i]++] = topselectedcard; // move to aces stack  
                       // remove selected card from open stack  
                       m_numberopenpack--;  
                       m_numberofselectedcards = 0;  
                       return TRUE;  
                   }  
               }  
               // next search 7 stacks  
               for (i = 0; i < 7; i++) {  
                   if (m_numbercard7stacks[i] > 0) {  
                       if (topselectedcard.id.cardnumber == m_card7stacks[i][m_numbercard7stacks[i] - 1].id.cardnumber - 1) { // selected card is the next card down on this stack  
                           if ((topselectedcard.id.suit < 2 && m_card7stacks[i][m_numbercard7stacks[i] - 1].id.suit >= 2) || (topselectedcard.id.suit >= 2 && m_card7stacks[i][m_numbercard7stacks[i] - 1].id.suit < 2))  
                           { // cards of different color can move card here  
                               SaveUndoRecord();  
                               topselectedcard.id.shown = 1; // card is shown.  
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
                       if (topselectedcard.id.cardnumber == 13) { // selected card is a king and this is an empty 7 stack  
                           SaveUndoRecord();  
                           topselectedcard.id.shown = 1; // card is shown.  
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

// MoveSelectedCardsToPt: Moves the selected cards to a specified point.  
// This function determines if the selected cards can be moved to a valid position  
// based on the rules of Solitaire. It checks for valid moves to 7-stacks or ace stacks.  
// If a valid move is found, the cards are moved, and the game state is updated.  
//  
// Parameters:  
// - topoint: A CPoint object representing the target location to move the cards.  
//  
// Returns:  
// - TRUE if the selected cards were successfully moved.  
// - FALSE if no valid move was found.  
BOOL CChildView::MoveSelectedCardsToPt(CPoint topoint)  
{
   int i, si, stack7, stackaces;
   CARDRESOURES topselectedcard, topcardtostack;
   if (m_numberofselectedcards > 0)
   {
       topselectedcard = m_selectedcards[0];
       stack7 = Get7StackUnderPt(topoint);
       if (stack7 >= 0) 
       {   // to point over 7 stack
           si = Get7StackWithImage(topselectedcard);
           if (si == stack7) {
               // same stack check for straight move
               if(MoveSelectedCard())
                   return TRUE;
           }
           if (m_numbercard7stacks[stack7] == 0) 
           {       // empty 7 stack
               if (topselectedcard.id.cardnumber == 13)
               {   // only kings can go onto an empty 7 stack
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
                   {   // selected cards came fron 7 stacks
                       m_numbercard7stacks[si] -= m_numberofselectedcards;
                       m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
                       m_numberofselectedcards = 0;    // unselect card
                       return TRUE;
                   }
                   else {
                       for (i = 0; i < m_numberopenpack; i++)
                       {       // open pack has cards open
                           if (m_openpack[i].img == topselectedcard.img) {
                               // this is the top selected card in the open stack
                               m_numberopenpack -= m_numberofselectedcards;
                               m_numberofselectedcards = 0;    // unselect card
                               return TRUE;
                           }
                       }
                       // check if card from aces pack
                       si = GetAceStackWithImage(topselectedcard);
                       if (si >= 0) {
                           // selected card on aces pack must be top card
                           m_numbercardaces[si]--;
                           m_numberofselectedcards = 0;    // unselect card
                           return TRUE;
                       }
                   }
               }
           } else {    // to  7 stack not empty
               topcardtostack = m_card7stacks[stack7][m_numbercard7stacks[stack7] - 1];
               if ((topcardtostack.id.cardnumber - topselectedcard.id.cardnumber) == 1)
               {   // next card number down.
                   // {_T("H"),_T("D"),_T("S"),_T("C")}
                   if ((topselectedcard.id.suit < 2 && topcardtostack.id.suit >= 2) || (topselectedcard.id.suit >= 2 && topcardtostack.id.suit < 2))
                   {   // cards of different color
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
                       {   // selected cards came fron 7 stacks
                           m_numbercard7stacks[si] -= m_numberofselectedcards;
                           m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
                           m_numberofselectedcards = 0;    // unselect card
                           return TRUE;
                       } else {
                           for(i=0;i< m_numberopenpack;i++)
                           {       // open pack has cards open
                               if (m_openpack[i].img == topselectedcard.img) {
                                   // this is the top selected card in the open stack
                                   m_numberopenpack -= m_numberofselectedcards;
                                   m_numberofselectedcards = 0;    // unselect card
                                   return TRUE;
                               }
                           }
                           // check if card from aces pack
                           si = GetAceStackWithImage(topselectedcard);
                           if (si >= 0) {
                               // selected card on aces pack must be top card
                               m_numbercardaces[si]--;
                               m_numberofselectedcards = 0;    // unselect card
                               return TRUE;
                           }
                       }                       
                   }
               }
           }
       }
       stackaces = GetAcesStackUnderPt(topoint);
       if (m_numberofselectedcards == 1 && stackaces >= 0) {    // only one card at a time on aces to point over aces stack
           if (m_numbercardaces[stackaces] == 0) 
           {       // empty ace stack
               if (topselectedcard.id.cardnumber == 1) 
               {   // only Aces can go onto an empty ace stack
                   si = Get7StackWithImage(topselectedcard);
                   SaveUndoRecord();
                   topselectedcard.id.shown = 1;
                   m_cardaces[stackaces][m_numbercardaces[stackaces]] = topselectedcard;
                   m_numbercardaces[stackaces]++;
                   // remove card from the from stack
                   if (si >= 0) {    // selected cards came fron 7 stacks
                       m_numbercard7stacks[si]--;
                       m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
                       m_numberofselectedcards = 0;    // unselect card
                       return TRUE;
                   }
                   else {
                       for (i = 0; i < m_numberopenpack; i++)
                       {       // open pack has cards open
                           if (m_openpack[i].img == topselectedcard.img) {
                               // this is the top selected card in the open stack
                               m_numberopenpack -= m_numberofselectedcards;
                               m_numberofselectedcards = 0;    // unselect card
                               return TRUE;
                           }
                       }
                   }
               }
           } else {    // to aces stack not empty
               topcardtostack = m_cardaces[stackaces][m_numbercardaces[stackaces] - 1];
               if ((topselectedcard.id.cardnumber - topcardtostack.id.cardnumber) == 1)
               {   // next card number up.
                   if (topselectedcard.id.suit == topcardtostack.id.suit)
                   {   // cards of same suit
                       si = Get7StackWithImage(topselectedcard);
                       SaveUndoRecord();
                       // move card to new stack
                       topselectedcard.id.shown = 1;
                       m_cardaces[stackaces][m_numbercardaces[stackaces]] = topselectedcard;
                       m_numbercardaces[stackaces]++;
                       // remove card from the from stack
                       if (si >= 0) {    // selected cards came fron 7 stacks
                           m_numbercard7stacks[si]--;
                           m_card7stacks[si][m_numbercard7stacks[si] - 1].id.shown = 1;
                           m_numberofselectedcards = 0;    // unselect card
                           return TRUE;
                       }
                       else {
                           for (i = 0; i < m_numberopenpack; i++)
                           {       // open pack has cards open
                               if (m_openpack[i].img == topselectedcard.img) {
                                   // this is the top selected card in the open stack
                                   m_numberopenpack--;
                                   m_numberofselectedcards = 0;    // unselect card
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

// GetSelectedCardsUnderPt: Determines which cards are selected under a given point.  
// This function checks the 7-stacks, open stack, and ace stacks to find cards under the specified point.  
// If a card is found, it is added to the selected cards array.  
//  
// Parameters:  
// - point: A CPoint object representing the coordinates to check.  
//  
// Returns:  
// - TRUE if one or more cards are selected under the point.  
// - FALSE if no cards are found under the point.  
BOOL CChildView::GetSelectedCardsUnderPt(CPoint point)  
{  
   int i, j, k;  
   CARDRESOURES card;  

   m_numberofselectedcards = 0;  
   card = GetCard7StackUnderPt(point); // Get card under point from 7 stacks.  
   if (card.id.cardnumber > 0) {  
       // Card selected from 7 stacks.  
       m_selectedcards[m_numberofselectedcards++] = card; // Move card to first selected.  
       // If card selected is not last in stack, find all following cards and select them all.  
       j = Get7StackUnderPt(point);  
       if (m_numbercard7stacks[j] > 1) {  
           // If the card is not the top one on the stack, copy all cards on the stack to selected.  
           if (m_card7stacks[j][0].img == card.img) {  
               // All stack selected, add any more cards.  
               for (k = 1; k < m_numbercard7stacks[j]; k++)  
                   m_selectedcards[m_numberofselectedcards++] = m_card7stacks[j][k];  

               return TRUE;  
           } else {  
               // Not all of stack selected.  
               for (i = m_numbercard7stacks[j] - 1; i > 0; i--) {  
                   if (m_card7stacks[j][i].img == card.img) {  
                       m_numberofselectedcards = 0; // Start selection from selected card and add next cards to selection.  
                       for (k = i; k < m_numbercard7stacks[j]; k++)  
                           m_selectedcards[m_numberofselectedcards++] = m_card7stacks[j][k];  
                       return TRUE;  
                   }  
               }  
           }  
       }  
       return TRUE;  
   }  
   card = GetOpenCardUnderPt(point); // Get card under point for open stack.  
   if (card.id.cardnumber > 0) {  
       // Card selected from open stack.  
       card.r = m_openpack[0].r; // Needs rectangle to drag.  
       card.id.shown = 1;  
       m_selectedcards[m_numberofselectedcards++] = card;  
       return TRUE;  
   }  
   card = GetAcesCardUnderPt(point); // Get card under point for ace stacks.  
   if (card.id.cardnumber > 0) {  
       // Card selected from ace stacks.  
       card.id.shown = 1;  
       m_selectedcards[m_numberofselectedcards++] = card;  
       return TRUE;  
   }  
   return FALSE;  
}

// OpenStackOneMore: Handles the logic for interacting with the closed stack.  
// This function checks if the user clicked on the closed stack and performs the following actions:  
// - If the closed stack is empty, it moves all cards from the open stack back to the closed stack.  
// - If the closed stack has cards, it moves the top card from the closed stack to the open stack.  
// The function also updates the game state and triggers a redraw of the affected areas.  
//  
// Parameters:  
// - point: A CPoint object representing the coordinates of the user's click.  
//  
// Returns:  
// - TRUE if a card was successfully moved or the stacks were updated.  
// - FALSE if no action was performed.  
BOOL CChildView::OpenStackOneMore(CPoint point)  
{  
   int i, j;  
   CRect rc;  
   // if over closed pack open card  
   rc = m_closedpack[0].r; // are we over the closed pack  
   if (rc.PtInRect(point)) {  
       if (m_numberclosedpack == 0) {  
           SaveUndoRecord();  
           // no cards in closed pack to turn over so put opened cards back  
           j = 0;  
           for (i = m_numberopenpack; i > 0; i--) {  
               m_closedpack[j++] = m_openpack[i - 1];  
           }  
           m_closedpack[0].r = rc; // put back the rectangle to first card on stack  
           m_numberclosedpack = m_numberopenpack;  
           m_numberopenpack = 0;  
           InvalidateRect(&m_openpack[0].r, TRUE);  
           InvalidateRect(&m_closedpack[0].r, TRUE);  
           return TRUE;  
       }  
       else {  
           // turn next card in closed pack to open pack  
           SaveUndoRecord();  
           rc = m_openpack[0].r; // save open pack rectangle  
           m_openpack[m_numberopenpack++] = m_closedpack[m_numberclosedpack - 1];  
           m_numberclosedpack--;  
           m_openpack[0].r = rc; // if this is first card opened then replace its closed rectangle with the open rectangle   
           InvalidateRect(&m_openpack[0].r, TRUE);  
           InvalidateRect(&m_closedpack[0].r, TRUE);  
           return TRUE;  
       }  
   }  
   return FALSE;  
}

// OnFileNewgame: Starts a new game by resetting the game state.
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

// OnKeyDown: Handles the key press events for the Solitaire game.  
// If the 'f' or 'F' key is pressed, it checks if all cards in the 7 stacks are turned over.  
// If so, it initiates the finishing sequence of the game by setting a timer to animate card movements.  
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

// OnTimer: Handles the WM_TIMER message to perform periodic actions.  
// This function is used to animate the finishing sequence of the game.  
// It invalidates the client area to trigger a redraw and stops the timer  
// when the finishing sequence is complete.  
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

// MoveCardToFinnish: Handles the logic for automatically moving cards to the ace stacks during the finishing phase of the game.  
// This function iterates through the ace stacks, 7-stacks, open stack, and closed stack to find cards that can be moved to the ace stacks.  
// It ensures that the cards are moved in the correct order based on the rules of Solitaire.  
//  
// Parameters:  
// - r: A pointer to a RECT structure that will be updated with the rectangle of the card being moved.  
//  
// Returns:  
// - TRUE if a card was successfully moved to an ace stack.  
// - FALSE if no valid move was found or the finishing phase is complete.  
BOOL CChildView::MoveCardToFinnish(RECT *r)  
{
   int i, ii, j;
   CARDRESOURES card, c;
   if (m_bFinnish)
   {
       // find card to move to aces stack
       for (i = 0; i < 4; i++)
       {
           if (m_numbercardaces[i] > 0) {
               // this ace stack has cards
               card = m_cardaces[i][m_numbercardaces[i]-1];    // top card on aces stack
               for (j = 0; j < 7; j++) {    // find a matching card on the 7 stack
                   if (m_numbercard7stacks[j]) {
                       c = m_card7stacks[j][m_numbercard7stacks[j]-1];    // top card
                       if (card.id.suit == c.id.suit) {
                           // 7 stack card suit matches aces suit
                           if (card.id.cardnumber == c.id.cardnumber - 1) {
                               // card can be moved
                               m_cardaces[i][m_numbercardaces[i]++] = c;    // move card to aces stack
                               m_numbercard7stacks[j]--;                    // remove card from 7 stack
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
                           m_cardaces[i][m_numbercardaces[i]++] = c;    // move card to aces stack
                           for (ii = j+1; ii < m_numberopenpack; ii++) {    // remove card from open stack
                               c = m_openpack[ii];        // card after card going to aces stack
                               m_openpack[ii-1]=c;        // shuffle down
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
                           m_cardaces[i][m_numbercardaces[i]++] = c;    // move card to aces stack
                           for (ii = j + 1; ii < m_numberclosedpack; ii++) {    // remove card from open stack
                               c = m_closedpack[ii];        // card after card going to aces stack
                               m_closedpack[ii - 1] = c;        // shuffle down
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
                       m_cardaces[i][m_numbercardaces[i]++] = m_closedpack[ii];    // move card to aces stack
                       m_numberclosedpack = ii;
                       // return rectangle of moved card
                       *r = m_openpack[0].r;
                       return TRUE;
                   }
               }
           }
       }    // for i=0;i<4
   }
   memset(r, 0, sizeof(RECT));
   return FALSE;
}

// OnEditUndo: Undoes the last move made by the player.
void CChildView::OnEditUndo()
{
	// player wants to undo last move.
	LoadUndoRecord();
	InvalidateRect(NULL, TRUE);
}

// read the 10 best times
void CChildView::GetBestTimes() 
{
    int             i,n,bt;
    CString         s,st;
    time_t          t;
    for (i = 0; i < 10; i++) {
        st.Format(_T("BestTime_%d"),i);
        s = AfxGetApp()->GetProfileString(_T("Settings"), st, _T(""));
        if (s.GetLength() > 1) {
            // record saved besttime, datetime
            n = s.Find(_T(","), 0);
            bt = _wtoi(s.Left(n));
            m_besttimes[i].m_besttime = bt;
            t = _wtoll(s.Right(s.GetLength() - (n+1)));
            m_besttimes[i].m_datetime = t;
        }
        else {
            m_besttimes[i].m_besttime = 0;
            m_besttimes[i].m_datetime = 0;
        }
    }
}

// write the 10 best times
void CChildView::WriteBestTimes()
{
    int             i;
    CString         s, st;
    for (i = 0; i < 10; i++) {
        st.Format(_T("BestTime_%d"), i);
        s.Format(_T("%d,%I64d"), m_besttimes[i].m_besttime, m_besttimes[i].m_datetime);
        AfxGetApp()->WriteProfileString(_T("Settings"), st, s);
    }
}

// TRUE if new best time, adds any new best time into m_besttimes and saves them
BOOL CChildView::IsNewBestTimes(int bt, time_t dt,int *id)
{
    int             i,j;
    BESTTIMERECORD  best;

    for (i = 0; i < 10; i++) {
        if (bt < m_besttimes[i].m_besttime || m_besttimes[i].m_besttime==0) {
            // this is a new best time insert besttime record in here
            for (j = 8; j >= i; j--) {
                best = m_besttimes[j];
                m_besttimes[j + 1] = best;
            }
            m_besttimes[i].m_besttime = bt;
            m_besttimes[i].m_datetime = dt;
            *id = i;            // index of this best time
            WriteBestTimes();
            return TRUE;
        }
    }
    return FALSE;
}


// OnHelpBesttimes: Displays the best times dialog.  
// This function creates an instance of the CBestTimesDlg dialog,  
// populates it with the best times data, and displays it modally.  
// After the dialog is closed, it is deleted to free resources.  
void CChildView::OnHelpBesttimes()  
{  
   int     i;  
   CBestTimesDlg* dlg = new CBestTimesDlg();  
   for (i = 0; i < 10; i++) {  
       dlg->m_besttimes[i] = m_besttimes[i];  
   }  
   dlg->DoModal();  
   delete dlg;  
}

// FinnishGame: Handles the logic for finishing the game.  
// This function is called when the game is completed or the player chooses to finish the game.  
// It calculates the time taken to finish the game, checks if the time qualifies as a new high score,  
// and updates the high score list if necessary.  
// Parameters: None  
// Returns: None  
void CChildView::FinnishGame()  
{  
   int             i;  
   time_t			tm;  

   m_gamefinnished = TRUE;  
   m_newhighscore = FALSE;  
   m_newscoreindex = -1;           // not a high score  
   if (m_timerstarted) {  
       time(&tm);  
       m_timerstarted = FALSE;		// game finnished stop timer  
       m_timetofinishgame = (int)(tm - m_startgametimer);      // number of seconds playing  
       if (IsNewBestTimes(m_timetofinishgame, tm, &i)) {  
           // i = index of new best time  
           m_highscore = m_timetofinishgame;  
           m_newscoreindex = i;  
           m_newhighscore = TRUE;  
           AfxGetApp()->WriteProfileInt(_T("Settings"), _T("HighScore"), m_highscore);	// save high score    
       }  
   }    
}

// DrawAcesPacks: Draws the four ace stacks on the game board.  
// This function iterates through the ace stacks and renders each stack.  
// If a stack is empty, it displays an "A" to indicate an empty ace stack.  
// If a stack contains cards, it draws the top card of the stack.  
// The function also updates the position of each ace stack for hit detection.  
//  
// Parameters:  
// - graphics: A pointer to the Gdiplus::Graphics object used for rendering.  
void CChildView::DrawAcesPacks(Gdiplus::Graphics *graphics)  
{  
   int         col,cwo,cho,x,y;  
   CString     s;  
   PointF		ptf;  
   SolidBrush	solidBrush(Color(255, 0, 0, 0));    // Black  

   s = _T("A");	    // write an A on each empty aces stack  
   cwo = (m_cardwidth / 2) - (int)(m_cardfont->GetSize() / 1.5);   // start character width offset  
   cho = (m_cardheight / 2) - (int)m_cardfont->GetSize();          // start character height offset  
   x = (m_cardmargin * 2) + m_cardwidth; y = m_cardmargin;	        // x,y = starting position on screen top,left  
   ptf.X = (REAL)x + cwo; ptf.Y = (REAL)y + cho;  
   for (col = 0; col < 4; col++)	// there are 4 aces stacks  
   {  
       graphics->DrawRectangle(m_cardrectpen, x, y, m_cardwidth, m_cardheight);  
       if (m_numbercardaces[col] == 0)		// pack empty  
           graphics->DrawString(s, s.GetLength(), m_cardfont, ptf, &solidBrush);	// "A"  
       else  
           graphics->DrawImage(m_cardaces[col][m_numbercardaces[col] - 1].img, x + 1, y + 1, m_cardwidth - 2, m_cardheight - 2);	// draw card on aces stack  
       m_cardaces[col][0].r.left = x; m_cardaces[col][0].r.right = x + m_cardwidth;	// save position of aces pack  
       m_cardaces[col][0].r.top = y; m_cardaces[col][0].r.bottom = y + m_cardheight;  
       x += m_cardwidth + m_cardmargin;		        // advance to next aces pack across  
       ptf.X = (REAL)x + cwo; ptf.Y = (REAL)y + cho;	// update string of next aces stack  
   }  
}

// DrawClosedPack: Draws the closed stack of cards on the game board.  
// This function calculates the position of the closed stack and renders it.  
// If there are cards in the closed stack, it displays the back of the top card.  
// If the stack is empty, it displays an empty card placeholder.  
// The function also updates the position of the closed stack for hit detection.
void CChildView::DrawClosedPack(Gdiplus::Graphics* graphics)
{
    int             x, y;
    SolidBrush		bkw(Color(255, 255, 255, 255));     // White
    x = (m_cardmargin * 2) + m_cardwidth; y = m_cardmargin;	// x,y = starting position on screen top,left
    x += (m_cardwidth + m_cardmargin) * 4;		        // advance 
    x += (m_cardwidth * 2) + (m_cardmargin * 4);		// calculate x position of closed stack
    if (m_numberclosedpack > 0)
        graphics->DrawImage(m_cardback, x + 1, y + 1, m_cardwidth - 2, m_cardheight - 2);	// still cards in closed pack to display, show card back.
    else
        graphics->FillRectangle(&bkw, x, y, m_cardwidth - 2, m_cardheight - 2);				// no closed cards left show empty card
    graphics->DrawRectangle(m_cardrectpen, x, y, m_cardwidth, m_cardheight);	// draw outline of closed card stack
    m_closedpack[0].r.left = x; m_closedpack[0].r.right = x + m_cardwidth;	// save position of closed pack
    m_closedpack[0].r.top = y; m_closedpack[0].r.bottom = y + m_cardheight;
}

// DrawOpenPack: Draws the open stack of cards on the game board.  
// This function calculates the position of the open stack and renders it.  
// If there are cards in the open stack, it displays the top card.  
// If the stack is empty, it displays an empty card placeholder.  
// The function also updates the position of the open stack for hit detection.
void CChildView::DrawOpenPack(Gdiplus::Graphics* graphics)
{
    int             x, y;
    SolidBrush		bkw(Color(255, 255, 255, 255));     // White

    x = (m_cardmargin * 2) + m_cardwidth; y = m_cardmargin;	// x,y = starting position on screen top,left
    x += (m_cardwidth + m_cardmargin) * 4;		            // advance 
    x += (m_cardwidth * 2) + (m_cardmargin * 4);		    // calculate x position of closed stack
    x -= m_cardmargin; y += m_cardheight + m_cardmargin;	// calculate x,y position of open stack

    if (m_numberopenpack > 0)
        graphics->DrawImage(m_openpack[m_numberopenpack - 1].img, x + 1, y + 1, m_cardwidth - 2, m_cardheight - 2);	// draw top card on open stack
    else
        graphics->FillRectangle(&bkw, x, y, m_cardwidth - 2, m_cardheight - 2);	// no open cards show empty card
    graphics->DrawRectangle(m_cardrectpen, x, y, m_cardwidth, m_cardheight);	// draw outline of open card stack
    m_openpack[0].r.left = x; m_openpack[0].r.right = x + m_cardwidth;	        // save position of open pack
    m_openpack[0].r.top = y; m_openpack[0].r.bottom = y + m_cardheight;
 }

// DrawStacksPacks: Draws the seven stacks of cards on the game board.  
// This function iterates through each column of the seven stacks and renders the cards.  
// If a card is shown, it displays the card image; otherwise, it displays the card back.  
// If a stack is empty, it displays an empty card placeholder.  
// The function also updates the position of each card for hit detection.  
//  
// Parameters:  
// - graphics: A pointer to the Gdiplus::Graphics object used for rendering.  
void CChildView::DrawStacksPacks(Gdiplus::Graphics* graphics)  
{  
   int             col, cnum, x, y;  
   SolidBrush		bkw(Color(255, 255, 255, 255));     // White  
   x = m_cardmargin; y = m_cardheight + (m_cardmargin * 2);	// x,y = starting position on screen top,left  
   for (col = 0; col < 7; col++) {		// for each column  
       y = m_cardheight + (m_cardmargin * 2);				    // calculate top for card 7 stacks  
       for (cnum = 0; cnum < m_numbercard7stacks[col]; cnum++) {	// for each card in column  
           graphics->DrawRectangle(m_cardrectpen, x, y, m_cardwidth, m_cardheight);  
           if (m_card7stacks[col][cnum].id.shown == 1)		// card shown  
               graphics->DrawImage(m_card7stacks[col][cnum].img, x + 1, y + 1, m_cardwidth - 2, m_cardheight - 2);	// draw shown card  
           else  
               graphics->DrawImage(m_cardback, x + 1, y + 1, m_cardwidth - 2, m_cardheight - 2);	// draw card back  
           m_card7stacks[col][cnum].r.left = x;                m_card7stacks[col][cnum].r.top = y;   
           m_card7stacks[col][cnum].r.right = x + m_cardwidth; m_card7stacks[col][cnum].r.bottom = y + m_cardheight;	// save position of card on 7 stack, needed for mouse point select  
           y = m_cardheight + (m_cardmargin * (cnum + 3));	// calculate next card down stack top  
       }  
       if (m_numbercard7stacks[col] == 0) {		// no cards in this 7 stack  
           graphics->FillRectangle(&bkw, x, y, m_cardwidth - 2, m_cardheight - 2);  
           graphics->DrawRectangle(m_cardrectpen, x, y, m_cardwidth, m_cardheight);  
       }  
       x += m_cardwidth + m_cardmargin;	// next column of 7 stacks  
   }  
}

// DisplayTimer: Displays the elapsed game time on the screen.  
// This function calculates the elapsed time since the game started and renders it  
// at the bottom-left corner of the screen. It clears the background of the timer  
// area before drawing the updated time.  
//  
// Parameters:  
// - graphics: A pointer to the Gdiplus::Graphics object used for rendering.  
void CChildView::DisplayTimer(Gdiplus::Graphics* graphics)  
{  
   int         tt;  
   CString     s;  
   time_t		tm;  
   PointF		ptf;  
   SolidBrush	solidBrush(Color(255, 0, 0, 0));    // Black  
   if (m_timerstarted) {  
       time(&tm);  
       tt = (int)(tm - m_startgametimer);     // total seconds  
       s.Format(_T("Time: %d:%02d"), tt / 60, tt % 60);  
       ptf.X = (REAL)m_cardmargin; ptf.Y = (REAL)m_screenheight - (m_cardfont->GetSize() * 2);     // bottom, left screen  
       graphics->FillRectangle(m_tablecolorbrush, ptf.X, ptf.Y, m_cardfont->GetSize() * 13, m_cardfont->GetSize() * 2);   // clear text background  
       graphics->DrawString(s, s.GetLength(), m_cardfont, ptf, &solidBrush); // display game time  
   }  
}