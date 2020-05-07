// Mainframe macro generated from application: /usr/ROOT/bin/root.exe
// By ROOT version 6.18/02 on 2020-01-16 19:26:24

#ifndef ROOT_TGDockableFrame
#include "TGDockableFrame.h"
#endif
#ifndef ROOT_TGMenu
#include "TGMenu.h"
#endif
#ifndef ROOT_TGMdiDecorFrame
#include "TGMdiDecorFrame.h"
#endif
#ifndef ROOT_TG3DLine
#include "TG3DLine.h"
#endif
#ifndef ROOT_TGMdiFrame
#include "TGMdiFrame.h"
#endif
#ifndef ROOT_TGMdiMainFrame
#include "TGMdiMainFrame.h"
#endif
#ifndef ROOT_TGMdiMenu
#include "TGMdiMenu.h"
#endif
#ifndef ROOT_TGListBox
#include "TGListBox.h"
#endif
#ifndef ROOT_TGNumberEntry
#include "TGNumberEntry.h"
#endif
#ifndef ROOT_TGScrollBar
#include "TGScrollBar.h"
#endif
#ifndef ROOT_TGComboBox
#include "TGComboBox.h"
#endif
#ifndef ROOT_TGuiBldHintsEditor
#include "TGuiBldHintsEditor.h"
#endif
#ifndef ROOT_TGuiBldNameFrame
#include "TGuiBldNameFrame.h"
#endif
#ifndef ROOT_TGFrame
#include "TGFrame.h"
#endif
#ifndef ROOT_TGFileDialog
#include "TGFileDialog.h"
#endif
#ifndef ROOT_TGShutter
#include "TGShutter.h"
#endif
#ifndef ROOT_TGButtonGroup
#include "TGButtonGroup.h"
#endif
#ifndef ROOT_TGCanvas
#include "TGCanvas.h"
#endif
#ifndef ROOT_TGFSContainer
#include "TGFSContainer.h"
#endif
#ifndef ROOT_TGuiBldEditor
#include "TGuiBldEditor.h"
#endif
#ifndef ROOT_TGColorSelect
#include "TGColorSelect.h"
#endif
#ifndef ROOT_TGButton
#include "TGButton.h"
#endif
#ifndef ROOT_TGFSComboBox
#include "TGFSComboBox.h"
#endif
#ifndef ROOT_TGLabel
#include "TGLabel.h"
#endif
#ifndef ROOT_TRootGuiBuilder
#include "TRootGuiBuilder.h"
#endif
#ifndef ROOT_TGTab
#include "TGTab.h"
#endif
#ifndef ROOT_TGListView
#include "TGListView.h"
#endif
#ifndef ROOT_TGSplitter
#include "TGSplitter.h"
#endif
#ifndef ROOT_TGStatusBar
#include "TGStatusBar.h"
#endif
#ifndef ROOT_TGListTree
#include "TGListTree.h"
#endif
#ifndef ROOT_TGuiBldGeometryFrame
#include "TGuiBldGeometryFrame.h"
#endif
#ifndef ROOT_TGToolTip
#include "TGToolTip.h"
#endif
#ifndef ROOT_TGToolBar
#include "TGToolBar.h"
#endif
#ifndef ROOT_TGuiBldDragManager
#include "TGuiBldDragManager.h"
#endif

#include "Riostream.h"

void test2()
{

   // main frame
   TGMainFrame *fMainFrame693 = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMainFrame693->SetName("fMainFrame693");
   fMainFrame693->SetLayoutBroken(kTRUE);
   TGTextButton *fTextButton560 = new TGTextButton(fMainFrame693,"fTextButton560",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
   fTextButton560->SetTextJustify(36);
   fTextButton560->SetMargins(0,0,0,0);
   fTextButton560->SetWrapLength(-1);
   fTextButton560->Resize(92,22);
   fMainFrame693->AddFrame(fTextButton560, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton560->MoveResize(40,32,92,22);
   TGTextButton *fTextButton565 = new TGTextButton(fMainFrame693,"fTextButton565",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
   fTextButton565->SetTextJustify(36);
   fTextButton565->SetMargins(0,0,0,0);
   fTextButton565->SetWrapLength(-1);
   fTextButton565->Resize(92,22);
   fMainFrame693->AddFrame(fTextButton565, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextButton565->MoveResize(56,72,92,22);
   TGLabel *fLabel570 = new TGLabel(fMainFrame693,"fLabel570");
   fLabel570->SetTextJustify(36);
   fLabel570->SetMargins(0,0,0,0);
   fLabel570->SetWrapLength(-1);
   fMainFrame693->AddFrame(fLabel570, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fLabel570->MoveResize(40,120,57,16);

   fMainFrame693->SetMWMHints(kMWMDecorAll,
                        kMWMFuncAll,
                        kMWMInputModeless);
   fMainFrame693->MapSubwindows();

   fMainFrame693->Resize(fMainFrame693->GetDefaultSize());
   fMainFrame693->MapWindow();
   fMainFrame693->Resize(490,372);
}  
