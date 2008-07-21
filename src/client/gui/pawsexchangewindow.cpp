/*
 * pawsexchangewindow.cpp - Author: Andrew Craig
 *
 * Copyright (C) 2001 Atomic Blue (info@planeshift.it, http://www.atomicblue.org) 
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation (version 2 of the License)
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <psconfig.h>
#include "globals.h"

////////////////////////////////////////////////////////////////////////////
//  COMMON INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "net/messages.h"
#include "net/msghandler.h"

/////////////////////////////////////////////////////////////////////////////
//  CLIENT INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "pawsexchangewindow.h"
#include "inventorywindow.h"

/////////////////////////////////////////////////////////////////////////////
//  PAWS INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "paws/pawscrollbar.h"
#include "paws/pawsmanager.h"
#include "paws/pawslistbox.h"
#include "gui/pawsmoney.h"
#include "pawsslot.h"


/////////////////////////////////////////////////////////////////////////////
//      TRADE SLOT NAMES
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


#define NUM_COLUMNS      2

/////////////////////////////////////////////////////////////////////////////
//  BUTTON DEFINES 
/////////////////////////////////////////////////////////////////////////////
#define EXCHANGE_ACCEPT                  100
/////////////////////////////////////////////////////////////////////////////

pawsExchangeWindow::pawsExchangeWindow()
{
}

pawsExchangeWindow::~pawsExchangeWindow()
{
    msgHandler->Unsubscribe( this, MSGTYPE_GUIEXCHANGE );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_REQUEST );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_ADD_ITEM );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_REMOVE_ITEM );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_ACCEPT );       
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_END );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_STATUS );
    msgHandler->Unsubscribe( this, MSGTYPE_EXCHANGE_MONEY );
}

bool pawsExchangeWindow::PostSetup()
{
    csString slotName;
    
    msgHandler = psengine->GetMsgHandler();
    if ( !msgHandler ) return false;

    if ( !msgHandler->Subscribe( this, MSGTYPE_GUIEXCHANGE ) ) return false;
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_REQUEST ) ) return false;
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_ADD_ITEM ) ) return false;
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_REMOVE_ITEM ) ) return false;
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_ACCEPT ) ) return false;        
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_END ) ) return false;        
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_STATUS ) ) return false;           
    if ( !msgHandler->Subscribe( this, MSGTYPE_EXCHANGE_MONEY ) ) return false;               
     
    // the two backgrounds for the offering/receiving frame
    offeringBG  = FindWidget("Offering Frame");
    receivingBG = FindWidget("Receiving Frame");
    if (!offeringBG || !receivingBG) 
        return false;
        
    offeringMoneyWidget   =  dynamic_cast <pawsMoney*> (FindWidget("OfferingMoney"));
    receivingMoneyWidget  =  dynamic_cast <pawsMoney*> (FindWidget("ReceivingMoney"));
               
    if (!offeringMoneyWidget || !receivingMoneyWidget) 
        return false;
    offeringMoneyWidget->SetContainer( CONTAINER_OFFERING_MONEY );    
    receivingMoneyWidget->SetContainer( CONTAINER_RECEIVING_MONEY );
    receivingMoneyWidget->Drag( false );

    totalTriasOffered = dynamic_cast <pawsTextBox*>(FindWidget("TotalTriasOffer"));
    totalTriasReceived= dynamic_cast <pawsTextBox*>(FindWidget("TotalTriasRec"));
    
    if(!totalTriasOffered || !totalTriasReceived)
        return false;
                
    // Create Offering bulk slots
    pawsListBox * bulkList = dynamic_cast <pawsListBox*> (FindWidget("OfferingList"));
    if (!bulkList)
        return false;

    int i, j;

    for (i = 0; i < EXCHANGE_SLOT_COUNT/NUM_COLUMNS; i++)
    {
        pawsListBoxRow * listRow = bulkList->NewRow(i);
        for (j = 0; j < NUM_COLUMNS; j++)
        {
            pawsSlot * slot;
            slot = dynamic_cast <pawsSlot*> (listRow->GetColumn(j));
            if (slot == NULL)
                return false;            
            slot->SetContainer( CONTAINER_EXCHANGE_OFFERING );            
            slot->SetSlotID( i*2+j );                   
            offeringSlots[i*2+j] = slot;            
        }
    }
    
    // Create Receiving bulk slots
    bulkList = (pawsListBox*)FindWidget("ReceivingList");
    if (!bulkList)
        return false;

    for (i = 0; i < EXCHANGE_SLOT_COUNT/NUM_COLUMNS; i++)
    {
        pawsListBoxRow * listRow = bulkList->NewRow(i);
        for (j = 0; j < NUM_COLUMNS; j++)
        {
            pawsSlot * slot;
            slot = dynamic_cast <pawsSlot*> (listRow->GetColumn(j));
            if (slot == NULL)
                return false;            
            slot->SetContainer( CONTAINER_EXCHANGE_RECEIVING );            
            slot->SetSlotID( i*2+j );                   
            slot->SetDrag(false);
            receivingSlots[i*2+j] = slot;               
        }
    }

    return true;
}

void pawsExchangeWindow::StartExchange( csString& player, bool withPlayer )
{
    csString text;
    if(!player.IsEmpty())
    {
        text.Format("Trading with %s",player.GetData());
    }

    int width;

    Clear();            
    Show();
            
    pawsTextBox* textBox = dynamic_cast <pawsTextBox*> (FindWidget("other_player"));
    if (textBox != NULL)
        textBox->SetText( text );

    if (withPlayer)
        width = 350;
    else
        width = 175;
    SetRelativeFrameSize(GetActualWidth(width), defaultFrame.Height());

    // Autoshow the inventory
    pawsWidget* widget = PawsManager::GetSingleton().FindWidget("SmallInventoryWindow");
    if ( widget )
        widget->Show();
}        


void pawsExchangeWindow::HandleMessage( MsgEntry* me )
{
    switch ( me->GetType() )    
    {
        /////////////////////////////////////
        //  Starting a new Exchange
        /////////////////////////////////////
        case MSGTYPE_EXCHANGE_REQUEST:
        {            
            psExchangeRequestMsg msg(me);
            StartExchange( msg.player, msg.withPlayer );
            break;    
        }
        
        ///////////////////////////////////////////////////////////
        //  Close Exchange ( either by rejection or normal end
        ///////////////////////////////////////////////////////////
        case MSGTYPE_EXCHANGE_END:
        {     
            Hide();
            pawsWidget * widget = PawsManager::GetSingleton().FindWidget("SmallInventoryWindow");
            if ( widget )
                widget->Close();
            totalTriasOffered->SetText("");
            totalTriasReceived->SetText("");
            int i;
            for (i = 0; i < EXCHANGE_SLOT_COUNT; i++)
            {                   
                offeringSlots[i]->Clear();
                receivingSlots[i]->Clear();
            }
            break;
        }
        
        ///////////////////////////////////////////////////////////
        //  Update the accept status for the exchange.
        ///////////////////////////////////////////////////////////        
        case MSGTYPE_EXCHANGE_STATUS:
        {
            psExchangeStatusMsg mesg(me);
            if ( mesg.playerAccept )
                offeringBG->SetBackground("Accepted Background");
            else 
                offeringBG->SetBackground("Standard Background");            
                
            if ( mesg.otherAccept )
                receivingBG->SetBackground("Accepted Background");                
            else
                receivingBG->SetBackground("Standard Background");
            break;
        }
        ///////////////////////////////////////////////////////////
        //  Incomming money update
        ///////////////////////////////////////////////////////////                        
        case MSGTYPE_EXCHANGE_MONEY:
        {
            HandleMoney( me );
            break;
        }
        
        ///////////////////////////////////////////////////////////
        //  An item has been removed from the exchange
        ///////////////////////////////////////////////////////////                
        case MSGTYPE_EXCHANGE_REMOVE_ITEM:
        {
            psExchangeRemoveItemMsg item(me);            
            pawsSlot* itemSlot = 0;
            
            if ( item.container == CONTAINER_EXCHANGE_OFFERING )
            {
                itemSlot = offeringSlots[item.slot];
            }
            else if ( item.container == CONTAINER_EXCHANGE_RECEIVING )
            {
                itemSlot = receivingSlots[item.slot];
            }
            
            if ( itemSlot )
            {
                itemSlot->StackCount(item.newStackCount);
                break;
            }
            
            break;
        }
        
        ///////////////////////////////////////////////////////////
        //  An item has been added to the exchange
        ///////////////////////////////////////////////////////////        
        case MSGTYPE_EXCHANGE_ADD_ITEM:
        {
            psExchangeAddItemMsg item(me);                        
            pawsSlot* itemSlot = 0;
            
            if ( item.container == CONTAINER_EXCHANGE_OFFERING )
            {
                itemSlot = offeringSlots[item.slot];
            }
            else if ( item.container == CONTAINER_EXCHANGE_RECEIVING )
            {
                itemSlot = receivingSlots[item.slot];
            }
            
            if ( itemSlot )
            {
                itemSlot->Clear();
                itemSlot->PlaceItem( item.icon, item.stackCount );
                itemSlot->SetToolTip( item.name );
                itemSlot->SetContainer( item.container );
                itemSlot->SetSlotID( item.slot );
            }

            break;
        }
    }
}

void pawsExchangeWindow::Close()
{
    pawsWidget * widget = PawsManager::GetSingleton().FindWidget("SmallInventoryWindow");
    if ( widget )
        widget->Close();

	totalTriasOffered->SetText("");
	totalTriasReceived->SetText("");
    
    SendEnd();
    Hide();        
}

bool pawsExchangeWindow::OnButtonPressed( int mouseButton, int keyModifier, pawsWidget* widget )
{
    csString widgetName(widget->GetName());
    
    if ( widgetName == "SmallInvButton" )
    {
        pawsWidget* widget = PawsManager::GetSingleton().FindWidget("SmallInventoryWindow");
        if ( widget )
            widget->Show();
        return true;        
    }
    
    if ( widget->GetID() == EXCHANGE_ACCEPT )
    {
        bool empty=true;
        for ( int x = 0; x < EXCHANGE_SLOT_COUNT; x++ )
        {
            if ( !offeringSlots[x]->IsEmpty() || 
                 !receivingSlots[x]->IsEmpty() )
                {
                  empty=false;  
                }
        }
        if ( offeringMoneyWidget->IsNoAmount() && 
            receivingMoneyWidget->IsNoAmount() && empty)
        {
           PawsManager::GetSingleton().CreateWarningBox("Maybe you want to actually give or get something before accepting the trade?");
           return false;
        }
        else
        {
            SendAccept();
            return true;
        }
    }

    return true;
}

void pawsExchangeWindow::HandleMoney( MsgEntry* me )
{
    psExchangeMoneyMsg money(me);
    
    if ( money.container == CONTAINER_OFFERING_MONEY )
    {
        offeringMoneyWidget->Set(money.circles, money.octas, money.hexas, money.trias);
        csString triasOffered;
        triasOffered.Format("Tria Offered %d", money.circles*CIRCLES_VALUE_TRIAS + money.octas*OCTAS_VALUE_TRIAS + money.hexas*HEXAS_VALUE_TRIAS + money.trias);
        totalTriasOffered->SetText(triasOffered);
    }
    else if ( money.container == CONTAINER_RECEIVING_MONEY )
    {
        receivingMoneyWidget->Set(money.circles, money.octas, money.hexas, money.trias);
        csString triasReceived;
        triasReceived.Format("Tria Received %d", money.circles*CIRCLES_VALUE_TRIAS + money.octas*OCTAS_VALUE_TRIAS + money.hexas*HEXAS_VALUE_TRIAS + money.trias);
        totalTriasReceived->SetText(triasReceived);
    }
}

void pawsExchangeWindow::SendAccept()
{    
    psExchangeAcceptMsg mesg;
    msgHandler->SendMessage(mesg.msg);
}

void pawsExchangeWindow::SendEnd()
{
    psExchangeEndMsg outgoing;
    msgHandler->SendMessage(outgoing.msg);
}


void pawsExchangeWindow::Clear()
{
    for ( int x = 0; x < EXCHANGE_SLOT_COUNT; x++ )
    {
        offeringSlots[x]->Clear();
		receivingSlots[x]->Clear();
    }        

    offeringMoneyWidget->Set(0,0,0,0);
    receivingMoneyWidget->Set(0,0,0,0);    
    
    offeringBG->SetBackground("Standard Background");            
    receivingBG->SetBackground("Standard Background");            
}

