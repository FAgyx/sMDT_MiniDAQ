// Mainframe macro generated from application: /usr/ROOT/bin/root.exe
// By ROOT version 6.18/02 on 2020-01-16 19:48:30

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
#ifndef ROOT_TGMsgBox
#include "TGMsgBox.h"
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

void GUI_test1()
{

   // main frame
   TGMainFrame *fMainFrame832 = new TGMainFrame(gClient->GetRoot(),10,10,kMainFrame | kVerticalFrame);
   fMainFrame832->SetName("fMainFrame832");
   fMainFrame832->SetLayoutBroken(kTRUE);

   // composite frame
   TGCompositeFrame *fMainFrame1536 = new TGCompositeFrame(fMainFrame832,580,431,kVerticalFrame);
   fMainFrame1536->SetName("fMainFrame1536");
   fMainFrame1536->SetLayoutBroken(kTRUE);
   TGTextButton *button1 = new TGTextButton(fMainFrame1536,"fTextButton1170",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
   button1->SetTextJustify(36);
   button1->SetMargins(0,0,0,0);
   button1->SetWrapLength(-1);
   button1->Resize(99,22);
   fMainFrame1536->AddFrame(button1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   button1->MoveResize(56,80,99,22);
   TGTextButton *button2 = new TGTextButton(fMainFrame1536,"fTextButton1297",-1,TGTextButton::GetDefaultGC()(),TGTextButton::GetDefaultFontStruct(),kRaisedFrame);
   button2->SetTextJustify(36);
   button2->SetMargins(0,0,0,0);
   button2->SetWrapLength(-1);
   button2->Resize(99,22);
   fMainFrame1536->AddFrame(button2, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   button2->MoveResize(56,136,99,22);

   TGFont *ufont;         // will reflect user font changes
   ufont = gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

   TGGC   *uGC;           // will reflect user GC changes
   // graphics context changes
   GCValues_t valEntry680;
   valEntry680.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
   gClient->GetColorByName("#000000",valEntry680.fForeground);
   gClient->GetColorByName("#e8e8e8",valEntry680.fBackground);
   valEntry680.fFillStyle = kFillSolid;
   valEntry680.fFont = ufont->GetFontHandle();
   valEntry680.fGraphicsExposures = kFALSE;
   uGC = gClient->GetGC(&valEntry680, kTRUE);
   TGTextEntry *fTextEntry680 = new TGTextEntry(fMainFrame1536, new TGTextBuffer(14),-1,uGC->GetGC(),ufont->GetFontStruct(),kSunkenFrame | kOwnBackground);
   fTextEntry680->SetMaxLength(4096);
   fTextEntry680->SetAlignment(kTextLeft);
   fTextEntry680->SetText("fTextEntry1302");
   fTextEntry680->Resize(92,fTextEntry680->GetDefaultHeight());
   fMainFrame1536->AddFrame(fTextEntry680, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextEntry680->MoveResize(240,56,92,20);

   ufont = gClient->GetFont("-*-helvetica-medium-r-*-*-12-*-*-*-*-*-iso8859-1");

   // graphics context changes
   GCValues_t valEntry681;
   valEntry681.fMask = kGCForeground | kGCBackground | kGCFillStyle | kGCFont | kGCGraphicsExposures;
   gClient->GetColorByName("#000000",valEntry681.fForeground);
   gClient->GetColorByName("#e8e8e8",valEntry681.fBackground);
   valEntry681.fFillStyle = kFillSolid;
   valEntry681.fFont = ufont->GetFontHandle();
   valEntry681.fGraphicsExposures = kFALSE;
   uGC = gClient->GetGC(&valEntry681, kTRUE);
   TGTextEntry *fTextEntry681 = new TGTextEntry(fMainFrame1536, new TGTextBuffer(14),-1,uGC->GetGC(),ufont->GetFontStruct(),kSunkenFrame | kOwnBackground);
   fTextEntry681->SetMaxLength(4096);
   fTextEntry681->SetAlignment(kTextLeft);
   fTextEntry681->SetText("fTextEntry1307");
   fTextEntry681->Resize(92,fTextEntry681->GetDefaultHeight());
   fMainFrame1536->AddFrame(fTextEntry681, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   fTextEntry681->MoveResize(248,104,92,20);
   TGNumberEntry *Number1 = new TGNumberEntry(fMainFrame1536, (Double_t) 0,6,-1,(TGNumberFormat::EStyle) 5);
   Number1->SetName("Number1");
   fMainFrame1536->AddFrame(Number1, new TGLayoutHints(kLHintsLeft | kLHintsTop,2,2,2,2));
   Number1->MoveResize(224,152,58,20);

   fMainFrame832->AddFrame(fMainFrame1536, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY));
   fMainFrame1536->MoveResize(0,0,580,431);

   fMainFrame832->SetMWMHints(kMWMDecorAll,
                        kMWMFuncAll,
                        kMWMInputModeless);
   fMainFrame832->MapSubwindows();

   fMainFrame832->Resize(fMainFrame832->GetDefaultSize());
   fMainFrame832->MapWindow();
   fMainFrame832->Resize(490,372);
}  
