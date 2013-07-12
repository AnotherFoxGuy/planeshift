/*
* shortcutwindow.cpp - Author: Andrew Dai
*
* Copyright (C) 2003 Atomic Blue (info@planeshift.it, http://www.atomicblue.org)
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

//=============================================================================
// Application Includes
//=============================================================================
#include "shortcutwindow.h"
#include "chatwindow.h"
#include "pscelclient.h"
#include "psclientchar.h"
#include "pawsscrollmenu.h"


//=============================================================================
// Defines
//=============================================================================
#define COMMAND_FILE            "/planeshift/userdata/options/shortcutcommands.xml"
#define DEFAULT_COMMAND_FILE    "/planeshift/data/options/shortcutcommands_def.xml"

#define BUTTON_PADDING          4
#define BUTTON_SPACING          8
#define WINDOW_PADDING          5
#define SCROLLBAR_SIZE          12

#define DONE_BUTTON             1100
#define CANCEL_BUTTON           1101
#define SETKEY_BUTTON           1102
#define CLEAR_BUTTON            1103
#define CLEAR_ICON_BUTTON       1104

#define SHORTCUT_BUTTON_OFFSET  2000
#define PALETTE_BUTTON_OFFSET  10000

//=============================================================================
// Classes
//=============================================================================

pawsShortcutWindow::pawsShortcutWindow()
{
    vfs =  csQueryRegistry<iVFS > ( PawsManager::GetSingleton().GetObjectRegistry());
    xml = psengine->GetXMLParser ();

    LoadCommandsFile();

    cmdsource = psengine->GetCmdHandler();
    chatWindow = (pawsChatWindow*)PawsManager::GetSingleton().FindWidget("ChatWindow");
    subWidget = NULL;
    shortcutText = NULL;
    textBox = NULL;
    labelBox = NULL;
}


pawsShortcutWindow::~pawsShortcutWindow()
{
    SaveCommands();
}

void pawsShortcutWindow::LoadCommandsFile()
{

// if there's a new style character-specific file then load it 
   csString CommandFileName,
             CharName( psengine->GetMainPlayerName() );
    size_t   spPos = CharName.FindFirst( ' ' );

    if( spPos != (size_t) -1 )
    { //there is a space in the name
        CommandFileName = CharName.Slice(0,spPos );
    }
    else
    {
        CommandFileName = CharName;
    }

    CommandFileName.Insert( 0, "/planeshift/userdata/options/shortcutcommands_" );
    CommandFileName.Append( ".xml" );
    if( vfs->Exists( CommandFileName.GetData() ))
    {
        LoadCommands(CommandFileName);
    }
    else
    {
        //if there's an old-style custom shortcuts file, load or append it
        if (vfs->Exists("/planeshift/userdata/options/shortcutcommands.xml"))
        {
            LoadCommands( "/planeshift/userdata/options/shortcutcommands.xml" );
        }
        else // if there's no customization then load the defaults
        {
            LoadCommands( "/planeshift/data/options/shortcutcommands_def.xml" );
        }

        //if there's an old-style quickbar config file, load it
        if(vfs->Exists("/planeshift/userdata/options/bartender.xml"))
        {
            LoadQuickbarFile();
        }

        SaveCommands();
    }
}

void pawsShortcutWindow::LoadQuickbarFile()
{
    //prepares to load the file
    csRef<iVFS> vfs =  csQueryRegistry<iVFS > ( PawsManager::GetSingleton().GetObjectRegistry());
    csRef<iDocumentSystem> xml = psengine->GetXMLParser ();
    csRef<iDocumentNode> root, bartenderNode;
    csRef<iDocument> doc = xml->CreateDocument();
    csRef<iDataBuffer> buf (vfs->ReadFile ("/planeshift/userdata/options/bartender.xml"));
    //if the file is empty or not opeanable we are done. We return true because it's not a failure.
    if (!buf || !buf->GetSize ())
    {
        return ;
    }
    const char* error = doc->Parse( buf );
    if ( error )
    {
        Error2("Error loading bartender entries: %s\n", error);
        return ;
    }

    root = doc->GetRoot();
    if (root == NULL)
    {
        Error2("%s has no XML root", "/planeshift/userdata/options/bartender.xml");
    }
    bartenderNode = root->GetNode("bartender");
    if (bartenderNode == NULL)
    {
        Error2("%s has no <bartender> tag", "/planeshift/userdata/options/bartender.xml");
    }

    //iterate all the nodes starting with slot in order to find the ones
    //to associate to the slots
    csRef<iDocumentNodeIterator> slotNodes = bartenderNode->GetNodes("slot");
    while (slotNodes->HasNext())
    {
        csRef<iDocumentNode> slotNode = slotNodes->Next();

        //get the data needed for the widget to be setup
        csString slotName = slotNode->GetAttributeValue("name");
        if( slotName.IsEmpty() )
        {
            names.Insert( 0, csString("") );
        }
        else
        {
            names.Insert(0, slotName);
        }

        csString slotImage = slotNode->GetAttributeValue("image");
        if( slotImage.Length()>0 ) // "(null)" == 6; any path we can use will be more...this is a cludge.
        {
            icon.Insert(0,slotImage);
        }
        else
        {
            icon.Insert( 0, csString("") );
        }

        csString slotAction = slotNode->GetAttributeValue("action");
        if( slotAction.IsEmpty() )
        {
            cmds.Insert( 0, csString("") );
        }
        else
        {
            cmds.Insert(0, slotAction);
        }
    }
    return ;
}

void pawsShortcutWindow::HandleMessage( MsgEntry* me )
{
}

bool pawsShortcutWindow::Setup(iDocumentNode *node)
{
    csRef<iDocumentNode> tempnode;
    csRef<iDocumentNodeIterator> nodeIter = node->GetNodes();
    while( nodeIter->HasNext() )
    {
        tempnode = nodeIter->Next();
        if( tempnode->GetAttributeValue("name") && strcmp( "MenuBar", tempnode->GetAttributeValue("name"))==0 )
        {
            csRef<iDocumentAttributeIterator> attiter = tempnode->GetAttributes();
            csRef<iDocumentAttribute> subnode;

            while ( attiter->HasNext() )
            {
                subnode = attiter->Next();
                if( strcmp( "buttonWidth", subnode->GetName() )==0 )
                {
                    if( strcmp( "auto", subnode->GetValue() )==0 )
                    {
                        buttonWidth=0;
                    }
                    else
                    {
                        buttonWidth=subnode->GetValueAsInt();
                    }
                }
                else if( strcmp( "scrollSize", subnode->GetName() )==0 )
                {
                    scrollSize=subnode->GetValueAsFloat();
                }
            }
            break;
        }
    }

    return true;
}


bool pawsShortcutWindow::PostSetup()
{
    size_t i;
    csArray<csString>  n;

    pawsControlledWindow::PostSetup();

    main_hp      = (pawsProgressBar*)FindWidget( "My HP" );
    main_mana    = (pawsProgressBar*)FindWidget( "My Mana" );
    phys_stamina = (pawsProgressBar*)FindWidget( "My PysStamina" );
    ment_stamina = (pawsProgressBar*)FindWidget( "My MenStamina" );
    if ( main_hp )
    {
        main_hp->SetTotalValue(1);
    }
    if ( main_mana )
    {
        main_mana->SetTotalValue(1);
    }
    if ( phys_stamina )
    {
        phys_stamina->SetTotalValue(1);
    }
    if ( ment_stamina )
    {
        ment_stamina->SetTotalValue(1);
    }
    if( main_hp || main_mana || phys_stamina || ment_stamina )
    {
        psengine->GetMsgHandler()->Subscribe(this,MSGTYPE_MODE);
    }

    MenuBar      = (pawsScrollMenu*)FindWidget( "MenuBar" );
    MenuBar->setButtonWidth( buttonWidth );
    if( scrollSize>1 )
    {
        MenuBar->setScrollIncrement( (int)scrollSize );
    }
    else
    {
        MenuBar->setScrollProportion( scrollSize );
    }
    //set to a default minimum size...expands as needed
    n =  names;
    for( i=0; i<names.GetSize(); i++ )
    {
        csString t = GetTriggerText( i );
        if( t.Length()>0 )
        {
            n[i].Insert( 0, " - " );
            n[i].Insert( 0, t );
        }
    }
    MenuBar->LoadArrays( names, icon, n, cmds, 2000, this);
    position = 0;

    return true;
}

void pawsShortcutWindow::OnResize()
{
}


bool pawsShortcutWindow::OnMouseDown( int button, int modifiers, int x, int y )
{
    if ( button == csmbWheelUp )
    {
	    return true;
    }
    else if ( button == csmbWheelDown )
    {
	    return true;
    }
    else
    {
	return pawsControlledWindow::OnMouseDown(button, modifiers, x, y);
    }
}


bool pawsShortcutWindow::OnButtonPressed( int mouseButton, int keyModifier, pawsWidget* widget )
{
    // Play a sound
    PawsManager::GetSingleton().GetSoundManager()->PlaySound("gui.shortcut", false, iSoundManager::GUI_SNDCTRL);
    return true;
}

bool pawsShortcutWindow::OnButtonReleased( int mouseButton, int keyModifier, pawsWidget* widget )
{
    if( ((pawsDnDButton* )widget)->IsDragDropInProgress() )
    {
        SaveCommands();
        ((pawsDnDButton*)widget)->SetDragDropInProgress( false );
        return true;
    }
    if(  psengine->GetSlotManager()->IsDragging() )
    {
        return true;
    }
    if (!subWidget)
        subWidget = PawsManager::GetSingleton().FindWidget("ShortcutEdit");

    if (!labelBox)
        labelBox = dynamic_cast <pawsEditTextBox*> (subWidget->FindWidget("LabelBox"));

    if (!textBox)
        textBox = dynamic_cast <pawsMultilineEditTextBox*> (subWidget->FindWidget("CommandBox"));

    if (!shortcutText)
        shortcutText = dynamic_cast <pawsTextBox*> (subWidget->FindWidget("ShortcutText"));

    if (!iconDisplay)
        iconDisplay = dynamic_cast <pawsDnDButton*> (subWidget->FindWidget("IconDisplay"));

    if (!iconPalette)
    {
        //for the icon Palette we don't want any names or actions, so we'll pass an empty stub array
        csArray<csString>	stubArray;
        iconPalette = dynamic_cast <pawsScrollMenu*> (subWidget->FindWidget("iconPalette"));

        //get a ptr to the txture manager so we can look at the elementList, which stores the icon names.
        pawsTextureManager *tm = PawsManager::GetSingleton().GetTextureManager();
        int i = tm->elementList.GetSize();
        
        //build an array of the icon names
        csHash<csRef<iPawsImage>, csString>::GlobalIterator Iter(tm->elementList.GetIterator());
        Iter.Reset();
        //build a lits of all PS icons
        while(Iter.HasNext())
        {
            allIcons.Push(  Iter.Next()->GetName() );
        }

        //pass the array of icon names to LoadArrays as both the icon and the tooltip, so we can see then names when we hover over one
        iconPalette->LoadArrays( stubArray, allIcons, allIcons, stubArray, PALETTE_BUTTON_OFFSET, this );
        iconPalette->SetEditLock( ScrollMenuOptionDISABLED );
        iconPalette->OnResize();

        iconDisplayID=-1;
    }

    // These should not be NULL
    CS_ASSERT(subWidget); CS_ASSERT(labelBox); CS_ASSERT(textBox); CS_ASSERT(shortcutText);

    switch ( widget->GetID() )
    {
        case DONE_BUTTON:
        {
            if (!labelBox->GetText() || *(labelBox->GetText()) == '\0' )
            {
                if (textBox->GetText() && *(textBox->GetText()) != '\0')
                {
                    //no name but a command was specified.  
                    psSystemMessage msg(0,MSG_ERROR,PawsManager::GetSingleton().Translate("Please specify a name when creating a shortcut."));
                    msg.FireEvent();
                    return true;
                }
                else //shortcut is empty and will be removed. Also remove possible key binding.
                {
                    //remove key bindings
                    csString editedCmd;
                    editedCmd.Format("Shortcut %d",edit+1);
                    psengine->GetCharControl()->RemapTrigger(editedCmd,psControl::NONE,0,0);

                    ((pawsDnDButton *)editedButton)->Clear();
                }
            }
            else  //labelBox (ie name) is set
            {
                if( edit < names.GetSize() )
                {
                    names[edit] = labelBox->GetText();
                    cmds[edit] = textBox->GetText();
                    if( iconDisplay->GetMaskingImageName().IsEmpty() )
                    {
                        icon[edit].Clear();
                    }
                    else
                    {
                        icon[edit] = iconDisplay->GetMaskingImageName();
                    }
                }
    	
                if( icon[edit].IsEmpty()>0 )
                {
                    editedButton->ClearMaskingImage();
                    ((pawsButton *)editedButton)->SetText( names[edit] );
                }
                else
                {
                    ((pawsDnDButton *)editedButton)->SetMaskingImage( icon[edit] );
                    ((pawsButton *)editedButton)->SetText( csString() );
                }

                csString t = GetTriggerText( edit );
                if( t.Length()>0 && edit < names.GetSize() )
                {
                    csString n = names[ edit ];
                    n.Insert( 0, " - " );
                    n.Insert( 0, t );
                    editedButton->SetToolTip( n );
                }
                else
                {
                    editedButton->SetToolTip( names[edit] );
                }

            } 
            SaveCommands();

            PawsManager::GetSingleton().SetModalWidget(NULL);
            PawsManager::GetSingleton().SetCurrentFocusedWidget(this);
            subWidget->Hide();

            Resize();
            ResetEditWindow();

            return true;
        }
        case CLEAR_BUTTON:
        {
            ResetEditWindow();

            return true;
        }
        case CLEAR_ICON_BUTTON:
        {
            iconDisplay->ClearMaskingImage();
            iconDisplay->SetMaskingImage( csString() );

            return true;
        }
        case CANCEL_BUTTON:
        {
            subWidget->Hide();
            ResetEditWindow();

            return true;
        }
        case SETKEY_BUTTON:
        {
            pawsWidget * fingWndAsWidget;
        
            fingWndAsWidget = PawsManager::GetSingleton().FindWidget("FingeringWindow");
            if (fingWndAsWidget == NULL)
            {
                Error1("Could not find widget FingeringWindow");
                return false;
            }
            pawsFingeringWindow * fingWnd = dynamic_cast<pawsFingeringWindow *>(fingWndAsWidget);
            if (fingWnd == NULL)
            {
                Error1("FingeringWindow is not pawsFingeringWindow");
                return false;
            }
            fingWnd->ShowDialog(this, labelBox->GetText());
    
            return true;
        }
    }            // switch( ... )
    if ( mouseButton == csmbLeft && !(keyModifier & CSMASK_CTRL))    //if left mouse button clicked
    {
        if( widget->GetID()>=PALETTE_BUTTON_OFFSET )        //if the clicked widget's offset is within the PALETTE range
        {
            iconDisplayID = widget->GetID();
            iconDisplay->SetMaskingImage(allIcons[iconDisplayID-PALETTE_BUTTON_OFFSET]);
	    icon[edit] = allIcons[iconDisplayID-PALETTE_BUTTON_OFFSET];
        }
        else
        {
            if( cmds.GetSize()>0 && !MenuBar->IsEditable() )
            {
                if( (cmds[widget->GetID() - SHORTCUT_BUTTON_OFFSET + position ]) ) 
                {
                    ExecuteCommand( widget->GetID() - SHORTCUT_BUTTON_OFFSET + position );
                }
            }
        }
    }
    else if ( mouseButton == csmbRight || (mouseButton == csmbLeft && (keyModifier & CSMASK_CTRL)) )
    {
        if( widget->GetID() >= PALETTE_BUTTON_OFFSET ) //ignore right-click on icon palette
        {
            return true;
        }

        if( !(MenuBar->IsEditable()) )
        {
            return false;
        }
        
        edit = widget->GetID() - SHORTCUT_BUTTON_OFFSET;
        editedButton = widget;

        if ( edit < 0 )
            return false;

        if (!subWidget || !labelBox || !textBox || !shortcutText)
            return false;

        if ( names[edit] && names[edit].Length() )
            labelBox->SetText( names[edit].GetData() );
        else
            labelBox->Clear();

        if ( cmds[edit] && cmds[edit].Length() )
        {
            textBox->SetText( cmds[edit].GetData() );
            shortcutText->SetText( GetTriggerText(edit) );
        }
        else
        {
            textBox->Clear();
            shortcutText->SetText("");
        }

        if( icon[edit] )
        {
            iconDisplay->SetMaskingImage(icon[edit]);
            iconDisplayID=edit+PALETTE_BUTTON_OFFSET;
        }
        else
        {
            iconDisplay->ClearMaskingImage();
        }

        subWidget->Show();
        PawsManager::GetSingleton().SetCurrentFocusedWidget(textBox);
    }
    else
    {
        return false;
    }
return true;
}

void pawsShortcutWindow::ResetEditWindow()
{
    iconDisplayID = -1;
    shortcutText->SetText( "" );
    iconDisplay->ClearMaskingImage();
    labelBox->Clear();
    textBox->Clear();
}


csString pawsShortcutWindow::GetTriggerText(int shortcutNum)
{
    psCharController* manager = psengine->GetCharControl();
    csString str;
    str.Format("Shortcut %d",shortcutNum+1);

    const psControl* ctrl = manager->GetTrigger( str );
    if (!ctrl)
    {
        printf("Unimplemented action '%s'!\n",str.GetData());
        return "";
    }

    return ctrl->ToString();
}


void pawsShortcutWindow::LoadDefaultCommands()
{
    LoadCommands(DEFAULT_COMMAND_FILE);
}


void pawsShortcutWindow::LoadCommands(const char * fileName)
{
    int number;
    // Read button commands
    csRef<iDocument> doc = xml->CreateDocument();
    
    csRef<iDataBuffer> buf (vfs->ReadFile (fileName));
    if (!buf || !buf->GetSize ())
    {
        return ;
    }
    const char* error = doc->Parse( buf );
    if ( error )
    {
        printf("Error loading shortcut window commands: %s\n", error);
        return ;
    }


    bool zerobased = false;

    csRef<iDocumentNodeIterator> iter = doc->GetRoot()->GetNode("shortcuts")->GetNodes();
    while ( iter->HasNext() )
    {
        csRef<iDocumentNode> child = iter->Next();

        if ( child->GetType() != CS_NODE_ELEMENT )
            continue;
        sscanf(child->GetValue(), "shortcut%d", &number);
        if(number == 0)
        {
            zerobased = true;
        }
        if(!zerobased)
        {
            number--;
        }
        if (number < 0 )
            continue;
        icon.Put(number, child->GetAttributeValue("buttonimage") );
        names.Put(number, child->GetAttributeValue("name") );
        cmds.Put(number, child->GetContentsValue() );
    }
    //set minimum sizes of command arrays
    if( names.GetSize()<NUM_SHORTCUTS )
    {
        names.SetSize( NUM_SHORTCUTS, csString( "") );
    }
    if( icon.GetSize()<NUM_SHORTCUTS )
    {
        icon.SetSize( NUM_SHORTCUTS, csString("") );
    }
    if( cmds.GetSize()<NUM_SHORTCUTS )
    {
        cmds.SetSize( NUM_SHORTCUTS, csString("") );
    }
}

void pawsShortcutWindow::SaveCommands(void)
{
    csString CommandFileName,
             CharName( psengine->GetMainPlayerName() );
    size_t   spPos = CharName.FindFirst( ' ' );

    if( spPos != (size_t) -1 )
    { //there is a space in the name
        CommandFileName = CharName.Slice(0,spPos );
    }
    else
    {
        CommandFileName = CharName;
    }

    CommandFileName.Insert( 0, "/planeshift/userdata/options/shortcutcommands_" );
    CommandFileName.Append( ".xml" );
    bool found = false;
    int i;
    for (i = 0;i < cmds.GetSize();i++)
    {
        if (cmds[i].IsEmpty())
            continue;

        found = true;
        break;
    }
    if (!found) // Don't save if no commands have been defined
        return ;

    // Save the commands with their labels
    csRef<iDocumentSystem> xml = csPtr<iDocumentSystem>(new csTinyDocumentSystem);
    csRef<iDocument> doc = xml->CreateDocument();
    csRef<iDocumentNode> root = doc->CreateRoot ();
    csRef<iDocumentNode> parentMain = root->CreateNodeBefore(CS_NODE_ELEMENT);
    parentMain->SetValue("shortcuts");
    csRef<iDocumentNode> parent;

    csRef<iDocumentNode> text;
    csString temp;
    for (i = 0;i < cmds.GetSize();i++)
    {
        if (cmds[i].IsEmpty())
            continue;
        parent = parentMain->CreateNodeBefore (CS_NODE_ELEMENT);
        temp.Format("shortcut%d", i + 1);
        parent->SetValue(temp);

        if (names[i].IsEmpty())
        {
            temp.Format("%d", i);
            parent->SetAttribute("name", temp);
        }
        else
        {
            parent->SetAttribute("name", names[i].GetData());
        }

        if (!icon[i].IsEmpty() )
        {
            parent->SetAttribute("buttonimage", icon[i].GetData());
        }
        text = parent->CreateNodeBefore(CS_NODE_TEXT);
        text->SetValue(cmds[i].GetData());
    }
    doc->Write(vfs, CommandFileName.GetData() );
}

        
void pawsShortcutWindow::ExecuteCommand(int shortcutNum )
{
    //if (shortcutNum < 0 || shortcutNum >= NUM_SHORTCUTS)
    if (shortcutNum < 0 )
    {
        return;
    }
    
    const char* current = cmds[shortcutNum].GetData();
    if (current)
    {
        const char* pos;
        const char* next = current - 1;
        csString command;
        
        while (next && *(next + 1))
        {
            // Move command pointer to start of next command
            pos = next + 1;
            command = pos;
            // Execute next command delimited by a line feed
            if (*pos)
            {
                // Find location of next command
                next = strchr(pos, '\n');
                if (next)
                    command.Truncate(next - pos);
            }

            if(command.GetAt(0) == '#') //it's a comment skip it
            {
                continue;
            }
 
            if(command.FindFirst("$target") != command.Length() - 1)
            {
                GEMClientObject *object= psengine->GetCharManager()->GetTarget();
                if(object)
                {
                    csString name = object->GetName(); //grab name of target
                    size_t space = name.FindFirst(" ");
                    name = name.Slice(0,space);
                    command.ReplaceAll("$target",name); // actually replace target
                }
            }
            if(command.FindFirst("$guild") != command.Length() - 1)
            {
                GEMClientActor *object= dynamic_cast<GEMClientActor*>(psengine->GetCharManager()->GetTarget());
                if(object)
                {
                    csString name = object->GetGuildName(); //grab guild name of target
                    command.ReplaceAll("$guild",name); // actually replace target
                }
            }
            if(command.FindFirst("$race") != command.Length() - 1)
            {
                GEMClientActor *object= dynamic_cast<GEMClientActor*>(psengine->GetCharManager()->GetTarget());
                if(object)
                {
                    csString name = object->race; //grab race name of target
                    command.ReplaceAll("$race",name); // actually replace target
                }
            }
            if(command.FindFirst("$sir") != command.Length() - 1)
            {
                GEMClientActor *object= dynamic_cast<GEMClientActor*>(psengine->GetCharManager()->GetTarget());
                if(object)
                {
                    csString name = "Dear";
                    switch (object->gender)
                    {
                    case PSCHARACTER_GENDER_NONE:
                        name = "Gemma";
                        break;
                    case PSCHARACTER_GENDER_FEMALE:
                        name = "Lady";
                        break;
                    case PSCHARACTER_GENDER_MALE:
                        name = "Sir";
                        break;
                    }
                    command.ReplaceAll("$sir",name); // actually replace target
                }
            }
            const char* errorMessage = cmdsource->Publish( command );
            if ( errorMessage )
                chatWindow->ChatOutput( errorMessage );
        }
    }
}


const csString& pawsShortcutWindow::GetCommandName(int shortcutNum )
{
    //if (shortcutNum < 0 || shortcutNum >= NUM_SHORTCUTS)
    if (shortcutNum < 0 )
    {
        static csString error("Out of range");
        return error;
    }

    if (!names[shortcutNum])
    {
        static csString unused("Unused");
        return unused;
    }
    
    return names[shortcutNum];
}


bool pawsShortcutWindow::OnFingering(csString string, psControl::Device device, uint button, uint32 mods)
{
    pawsFingeringWindow* fingWnd = dynamic_cast<pawsFingeringWindow*>(PawsManager::GetSingleton().FindWidget("FingeringWindow"));
    if (fingWnd == NULL || !fingWnd->IsVisible())
        return true;

    csString editedCmd;
    editedCmd.Format("Shortcut %d",edit+1);

    bool changed = false;

    if (string == NO_BIND)  // Removing trigger
    {
        psengine->GetCharControl()->RemapTrigger(editedCmd,psControl::NONE,0,0);
        changed = true;
    }
    else  // Changing trigger
    {
        changed = psengine->GetCharControl()->RemapTrigger(editedCmd,device,button,mods);
    }

    if (changed)
    {
        shortcutText->SetText(GetTriggerText(edit));
        return true;  // Hide fingering window
    }
    else  // Map already exists
    {
        const psControl* other = psengine->GetCharControl()->GetMappedTrigger(device,button,mods);
        CS_ASSERT(other);

        csString name = GetDisplayName(other->name);
        if (name.IsEmpty())     //then not cleaned up properly from deleting a shortcut
        {
            name = other->name; //use its numeric name
        }
        else
        {
            name.AppendFmt(" (%s)", other->name.GetDataSafe());
        }

        fingWnd->SetCollisionInfo(name);

        return false;
    }
}


void pawsShortcutWindow::Show()
{
    psStatDRMessage msg;
    msg.SendMessage();
    pawsControlledWindow::Show();
}

