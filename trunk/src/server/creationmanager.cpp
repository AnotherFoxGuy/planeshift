/*
 * creationmanager.cpp - author: Andrew Craig
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

//=============================================================================
// Library Includes
//=============================================================================
#include "util/psdatabase.h"
#include "util/serverconsole.h"
#include "util/eventmanager.h"

#include "bulkobjects/psraceinfo.h"
#include "bulkobjects/pscharacterloader.h"

#include "net/msghandler.h"
#include "net/charmessages.h"

//=============================================================================
// Application Includes
//=============================================================================
#include "psserver.h"
#include "globals.h"
#include "creationmanager.h"
#include "client.h"
#include "clients.h"
#include "psserverchar.h"
#include "entitymanager.h"
#include "progressionmanager.h"
#include "cachemanager.h"
#include "iserver/idal.h"


// The number of characters per email account
#define CHARACTERS_ALLOWED 4   

//////////////////////////////////////////////////////////////////////////////
// SFC MACROS
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////



psCharCreationManager::psCharCreationManager()
{    
    raceCPValues = 0;
    raceCPValuesLength = 0;
    if (psserver->GetEventManager())
        psserver->GetEventManager()->Unsubscribe(this, MSGTYPE_CHAR_CREATE_UPLOAD);
}

psCharCreationManager::~psCharCreationManager()
{
    delete[] raceCPValues;    
}


bool psCharCreationManager::Initialize( )
{

    if ( !( LoadCPValues() && 
            LoadCreationChoices() && 
            LoadLifeEvents()) )
        return false;
        
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_UPLOAD,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_CP,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_PARENTS,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_CHILDHOOD,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_LIFEEVENTS,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_TRAITS,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_CREATE_NAME,REQUIRE_ANY_CLIENT);
    psserver->GetEventManager()->Subscribe(this, MSGTYPE_CHAR_DELETE,REQUIRE_ANY_CLIENT);
   
    // Other loaders are here.        
    return true;
}

void psCharCreationManager::HandleMessage(MsgEntry *pMsg,Client *client)
{
    switch ( pMsg->GetType() )
    {
        /* This is a simple message requesting a CP value for a race
         * so we can handle the raw message data here.
         */
        case MSGTYPE_CHAR_CREATE_CP:
        {
            HandleCharCreateCP( pMsg, client );
            return;
        }  
       
        case MSGTYPE_CHAR_DELETE:
        {
            HandleCharDelete( pMsg, client );
            return;
        }

        case MSGTYPE_CHAR_CREATE_NAME:
        {
            HandleName(pMsg,client);
            return;
        }
        case MSGTYPE_CHAR_CREATE_PARENTS:
        {
            HandleParents(pMsg,client);
            return;            
        }
        
        case MSGTYPE_CHAR_CREATE_CHILDHOOD:
        {
            HandleChildhood(pMsg,client);
            return;
        }

        case MSGTYPE_CHAR_CREATE_LIFEEVENTS:
        {
            HandleLifeEvents(pMsg,client);
            return;
        }
      
        case MSGTYPE_CHAR_CREATE_TRAITS:
        {
            HandleTraits(pMsg,client);
            return;
        }
                    
        case MSGTYPE_CHAR_CREATE_UPLOAD: 
        { 
            HandleUploadMessage( pMsg, client ); 
            break;
        } 
    }
}

bool psCharCreationManager::HandleCharDelete( MsgEntry* me, Client* client )
{
    psCharDeleteMessage msg(me);
    csString charName = msg.charName;

    if(!charName.Length())
    {
        CPrintf(CON_WARNING,"Client %u sent malformed character name to deletion code!",client->GetClientNum());
        return false; // No char
    }

    unsigned int characteruid = psserver->CharacterLoader.FindCharacterID( client->GetAccountID(), charName );
    csString error;

    if ( psserver->CharacterLoader.AccountOwner( charName, client->GetAccountID() ) )
    {
        // Found the char?
        if ( !characteruid)
        {
            psserver->SendSystemError(client->GetClientNum(),"Couldn't find character data!");
            return false;
        }

        // Can we delete it?
        if (!psserver->CharacterLoader.DeleteCharacterData(characteruid,error))
        {
            psserver->SendSystemError(client->GetClientNum(),"Error: %s",error.GetData());
            return false;
        }

        // Remove cached objects to make sure that the client gets a fresh character
        // list from the database.
        iCachedObject *obj = CacheManager::GetSingleton().RemoveFromCache(CacheManager::GetSingleton().MakeCacheName("list",client->GetAccountID()));
        if (obj)
        {
            obj->ProcessCacheTimeout();
            obj->DeleteSelf();
        }
        obj = CacheManager::GetSingleton().RemoveFromCache(CacheManager::GetSingleton().MakeCacheName("auth",client->GetAccountID()));
        if (obj)
        {
            obj->ProcessCacheTimeout();
            obj->DeleteSelf();
        }

        // Ok, we deleted it
        psCharDeleteMessage response( charName, me->clientnum );
        response.SendMessage();
    }
    else
    {
        CPrintf(CON_WARNING,"Character %s not deleted because of non-ownership\n", charName.GetData() );
        psserver->SendSystemError(client->GetClientNum(),"You do not own that character!");
        return false;
    }

    return true;
}


bool psCharCreationManager::LoadCPValues()
{
    Result result(db->Select("SELECT id,initial_cp from race_info"));
    if ( !result.IsValid() || result.Count() == 0 )
        return false;
    
    raceCPValuesLength = result.Count();
    raceCPValues = new RaceCP[ raceCPValuesLength ];
    
    for (unsigned int x = 0; x < result.Count(); x++ )
    {
        raceCPValues[x].id    = result[x].GetInt(0);
        raceCPValues[x].value = result[x].GetInt(1);
    }
    
    return true;     
}

bool psCharCreationManager::LoadLifeEvents()
{
    Result events( db->Select("SELECT * from char_create_life") );
    
    if ( !events.IsValid() || events.Count() == 0 )
        return false;

    for ( unsigned int x = 0; x < events.Count(); x++ )
    {
        LifeEventChoiceServer* choice = new LifeEventChoiceServer;
        choice->id = events[x].GetInt(0);
        choice->name = events[x][1];
        choice->description = events[x][2];
        choice->cpCost = events[x].GetInt(3);
        choice->eventScript = events[x][4];
        
        csString common = events[x][5];
        choice->common = common.GetAt(0);
        
        Result adds( db->Select("SELECT adds_choice from char_create_life_relations WHERE adds_choice IS NOT NULL AND choice=%d", choice->id) );
        if (!adds.IsValid())
            return false;

        for ( unsigned int addIndex = 0; addIndex < adds.Count(); addIndex++ )
        {
            choice->adds.Push( adds[addIndex].GetInt(0) );
        }
        
        Result removes( db->Select("SELECT removes_choice from char_create_life_relations WHERE removes_choice IS NOT NULL AND choice=%d", choice->id) );
        if (!removes.IsValid())
            return false;

        for ( unsigned int removesIndex = 0; removesIndex < removes.Count(); removesIndex++ )
        {
            choice->removes.Push( removes[removesIndex].GetInt(0) );
        }
        
        lifeEvents.Push( choice );
    }  
    
    return true;                      
}

bool psCharCreationManager::LoadCreationChoices()
{
    Result result( db->Select("SELECT * from character_creation") );
    
    if ( !result.IsValid() || result.Count() == 0 )
        return false;
               
    /* For all the possible choices:    
        Create them 
        Find out into what data set they should be pushed
     */
    for ( unsigned int x = 0; x < result.Count(); x++ )
    {
        CreationChoice* choice = new CreationChoice;
        choice->id   = result[x].GetInt(0);
        choice->name = result[x][1];
        choice->description = result[x][2];
        choice->cpCost = result[x].GetInt(3);
        choice->eventScript = result[x][4];
        choice->choiceArea = ConvertAreaToInt( result[x][5] );
        
        switch (choice->choiceArea)
        {
            case FATHER_JOB:
            case MOTHER_JOB:
            case RELIGION:
            {
                parentData.Push( choice );
                break;
            }
            
               
            case BIRTH_EVENT:
            case CHILD_ACTIVITY:
            case CHILD_HOUSE:            
            case CHILD_SIBLINGS:
            case ZODIAC:
            {
                childhoodData.Push( choice );
                break;
            }
        }
    }   
    
    return true;             
}
 
 
int psCharCreationManager::ConvertAreaToInt( const char* area )
{
    csString str( area );
    
    if ( str == "ZODIAC" )
        return ZODIAC;
    if ( str == "FATHER_JOB" )
        return FATHER_JOB;
        
    if ( str == "MOTHER_JOB" ) 
        return MOTHER_JOB;        
    
    if ( str == "RELIGION" ) 
        return RELIGION;   
           
    if ( str == "BIRTH_EVENT" )
        return BIRTH_EVENT;
        
    if ( str == "CHILD_ACTIVITY" )
        return CHILD_ACTIVITY;
        
    if ( str == "CHILD_HOUSE" )
        return CHILD_HOUSE;
    
    if ( str == "CHILD_SIBLINGS" )
        return CHILD_SIBLINGS;
                
    return -1;
}

void psCharCreationManager::HandleName( MsgEntry* me, Client *client )
{
    psNameCheckMessage name;
    name.FromClient( me );
    /// Check in migration/reserve table
    csString query;
    csString escape;
    if ( name.firstName.Length() == 0 )
    {
        psNameCheckMessage response( me->clientnum, false, "Cannot have an empty first name!");      
        response.SendMessage();
        return;
    }

    if (name.firstName.Length() > 27)
    {
        psNameCheckMessage response(me->clientnum, false, "First name is too long!");
        response.SendMessage();
        return;
    }

    if (name.lastName.Length() > 27)
    {
        psNameCheckMessage response(me->clientnum, false, "Last name is too long!");
        response.SendMessage();
        return;
    }

    db->Escape( escape, name.firstName );
    query.Format( "SELECT * FROM migration m WHERE m.username='%s'", escape.GetData() );    
    Result result (db->Select( query ) );    
    if ( result.IsValid() && result.Count() == 1 )
    {
        int acctID = client->GetAccountID();
        int reservedName = IsReserved( name.firstName, acctID );
        if ( reservedName == NAME_RESERVED )
        {
            psNameCheckMessage response( me->clientnum, false, "Name is in reserved database"); 
            response.SendMessage();
            return;
        }            
    }
    
    // Check uniqueness of the names
    if(!IsUnique(name.firstName))
    {
        psNameCheckMessage response( me->clientnum, false, "First name is already in use"); 
        response.SendMessage();        
        return;
    }

    if(!IsLastNameUnique(name.lastName))
    {
        psNameCheckMessage response( me->clientnum, false, "Last name is already in use"); 
        response.SendMessage();        
        return;
    }

    csString playerName = name.firstName;
    csString lastName   = name.lastName;

    // Check if the name is banned
    if(psserver->GetCharManager()->IsBanned(playerName))
    {
        csString error;
        error.Format("The name %s is banned", playerName.GetData() );
        psNameCheckMessage reject(me->clientnum,
                                    false,
                                    error );
        reject.SendMessage();
        return;
    }
    
    if(psserver->GetCharManager()->IsBanned(lastName))
    {
        csString error;
        error.Format("The lastname %s is banned", lastName.GetData() );
        psNameCheckMessage reject(me->clientnum,
                                    false,
                                    error );
        reject.SendMessage();
        return;
    }

    psNameCheckMessage response( me->clientnum, true, "Ok"); 
    response.SendMessage();                                            
}

void psCharCreationManager::HandleChildhood( MsgEntry* me,Client *client )
{
    psCreationChoiceMsg message(me);
    
    if ( !message.valid )
    {
        Debug2( LOG_NET, me->clientnum,"Received unparsable Childhood request from client: %u\n", me->clientnum );
        return;
    }
    
    psCreationChoiceMsg response( me->clientnum, (int) childhoodData.GetSize(), MSGTYPE_CHAR_CREATE_CHILDHOOD  );
    
    for (size_t x = 0; x < childhoodData.GetSize(); x++ )
    {
        response.AddChoice( childhoodData[x]->id, 
                            childhoodData[x]->name,
                            childhoodData[x]->description,
                            childhoodData[x]->choiceArea,
                            childhoodData[x]->cpCost );
    }
    
    response.ConstructMessage();
    if (response.valid)
    {
        response.SendMessage();
    }
    else
    {
        Bug2("Failed to construct a valid psCreationChoiceMsg for client id %u.\n",me->clientnum);
    }        
    
}

void psCharCreationManager::HandleParents( MsgEntry* me,Client *client )
{
    psCreationChoiceMsg message(me);
    if (!message.valid)
    {
        Debug2(LOG_NET,me->clientnum,"Received unparsable psCreationChoiceMsg from client id %u.\n",me->clientnum);
        return;
    }
        
    psCreationChoiceMsg response( me->clientnum, (int) parentData.GetSize(), MSGTYPE_CHAR_CREATE_PARENTS  );
    for (size_t x = 0; x < parentData.GetSize(); x++ )
    {
        response.AddChoice( parentData[x]->id, 
                            parentData[x]->name,
                            parentData[x]->description,
                            parentData[x]->choiceArea,
                            parentData[x]->cpCost );
    }
    
    response.ConstructMessage();
    if (response.valid)
        response.SendMessage();
    else
    {
        Bug2("Failed to construct a valid psCreationChoiceMsg for client id %u.\n",me->clientnum);
    }
}

void psCharCreationManager::HandleLifeEvents( MsgEntry* me,Client *client )
{
    psLifeEventMsg message(me);

    if (!message.valid)
    {
        Debug2(LOG_NET,me->clientnum,"Received unparsable psLifeEventMsg from client id %u.\n",me->clientnum);
        return;
    }

    psLifeEventMsg response( me->clientnum );

    for (size_t x = 0; x < lifeEvents.GetSize(); x++ )
    {
        response.AddEvent( lifeEvents[x] );
    }

    response.ConstructMessage();
    if (response.valid)
        response.SendMessage();
    else
    {
        Bug2("Failed to construct a valid psLifeEventMsg for client id %u.\n",me->clientnum);
    }

}

void psCharCreationManager::HandleTraits( MsgEntry* me,Client *client )
{
    psCreationChoiceMsg message(me);

    if (!message.valid)
    {
        Debug2(LOG_NET,me->clientnum,"Received unparsable psCreationChoiceMsg from client id %u.\n",me->clientnum);
        return;
    }

   csString str = "<traits>";
   CacheManager::TraitIterator traits = CacheManager::GetSingleton().GetTraitIterator();
   while (traits.HasNext())
   {
      psTrait * trait = traits.Next();
      if (!trait->onlyNPC)
      {
         str += trait->ToXML();
      }
   }
   str += "</traits>";

   // printf("Handle traits: %s\n",str.GetData());

   psCharCreateTraitsMessage response(client->GetClientNum(), str);
   response.SendMessage();
}



psCharCreationManager::LifeEventChoiceServer* psCharCreationManager::FindLifeEvent( int id )
{
    for (size_t x = 0; x < lifeEvents.GetSize(); x++ )
    {
        if ( lifeEvents[x]->id == id )
            return lifeEvents[x];
    }
    
    return 0;
}


psCharCreationManager::CreationChoice* psCharCreationManager::FindChoice( int id )
{
    size_t x;
    for ( x = 0; x < childhoodData.GetSize(); x++ )
    {
        if ( childhoodData[x]->id == id )
            return childhoodData[x];
    }
    
    for ( x = 0; x < parentData.GetSize(); x++ )
    {
        if ( parentData[x]->id == id )
            return parentData[x];
    }
    
    return 0;
}



bool psCharCreationManager::Validate( psCharUploadMessage& mesg, csString& errorMsg )
{

    // Check for bad parent modifier.    
    if ( mesg.fatherMod > 3 || mesg.fatherMod < 1 )
    {   
        Notify1( LOG_NEWCHAR, "New character tried to use invalid father modification" );
        errorMsg = "Invalid father modification selected";
        return false;
    }    
        
    if ( mesg.motherMod > 3 || mesg.motherMod < 1 )
    {
        Notify1( LOG_NEWCHAR, "New character tried to use invalid mother modification" );    
        errorMsg = "Invalid mother modification selected";
        return false;
    }        
    
    int cpCost = CalculateCPLife( mesg.lifeEvents ) + 
                 CalculateCPChoices( mesg.choices, mesg.fatherMod, mesg.motherMod );
                 
     psRaceInfo* race;
     race = CacheManager::GetSingleton().GetRaceInfoByNameGender( mesg.race, (PSCHARACTER_GENDER)mesg.gender );                      

    if ( !race )
    {
        errorMsg = "No race selected";
        return false;
    }
    else
    {
        if ( cpCost > race->initialCP )
        {
            Notify1( LOG_NEWCHAR, "New character exceeded CP allowance" );   
            errorMsg = "CP allowance exceeded.";
            return false;
        }            
    }                                                            
    return true;
}


int psCharCreationManager::CalculateCPLife( csArray<uint32_t>& events )
{
    int cpCost = 0;
    for ( size_t li = 0; li < events.GetSize(); li++ )
    {
        psCharCreationManager::LifeEventChoiceServer *event;
        event = psserver->charCreationManager->FindLifeEvent( events[li] );
        
        if ( event )
        {
            cpCost+=event->cpCost;
        }        
    }    
    
    return cpCost;
}

int psCharCreationManager::CalculateCPChoices( csArray<uint32_t>& choices, int fatherMod, int motherMod )
{
    int cpCost = 0;
    for ( size_t ci = 0; ci < choices.GetSize(); ci++ )
    {
        psCharCreationManager::CreationChoice* choice = FindChoice( choices[ci] );
        if ( choice )
        {
            if ( choice->choiceArea == FATHER_JOB )
            {
                cpCost+= choice->cpCost*fatherMod;
            }
            else if ( choice->choiceArea == MOTHER_JOB )
            {
                cpCost+= choice->cpCost*motherMod;
            }    
            else
                cpCost+= choice->cpCost;        
        }        
    }
    
    return cpCost;
}

void psCharCreationManager::HandleUploadMessage( MsgEntry* me, Client *client )
{
    Debug1( LOG_NEWCHAR, me->clientnum,"New Character is being created" );

    psCharUploadMessage upload(me);

    if (!upload.valid)
    {
        Debug2(LOG_NET,me->clientnum,"Received unparsable psUploadMessage from client %u.",me->clientnum);
        return;
    }

    int acctID = client->GetAccountID();
    if ( !acctID )
    {
        Error2( "Player account %d could not be located", acctID );

        psCharRejectedMessage reject(me->clientnum);

        psserver->GetEventManager()->Broadcast(reject.msg, NetBase::BC_FINALPACKET);
        psserver->RemovePlayer (me->clientnum,"Could not find your account.");
        return;
    }

    // Check to see if the player already has 4 accounts;
    csString query;
    query.Format( "Select id from characters where account_id=%d", acctID );
    Result result (db->Select( query ) );
    if ( result.IsValid() && result.Count() >= CHARACTERS_ALLOWED )
    {        
        psserver->RemovePlayer (me->clientnum,"At your character limit.");
        return;
    }
        
    csString playerName =  upload.name;
    csString lastName =  upload.lastname;

    playerName = NormalizeCharacterName(playerName);
    lastName = NormalizeCharacterName(lastName);

    // Check banned names
    if(psserver->GetCharManager()->IsBanned(playerName))
    {
        csString error;
        error.Format("The name %s is banned", playerName.GetData() );
        psCharRejectedMessage reject(me->clientnum,
                            psCharRejectedMessage::RESERVED_NAME,
                            (char*)error.GetData() );
        reject.SendMessage();
        return;
    }

    if(psserver->GetCharManager()->IsBanned(lastName))
    {
        csString error;
        error.Format("The lastname %s is banned", lastName.GetData() );
        psCharRejectedMessage reject(me->clientnum,
                            psCharRejectedMessage::RESERVED_NAME,
                            (char*)error.GetData() );
        reject.SendMessage();
        return;
    }

    Debug3( LOG_NEWCHAR, me->clientnum,"Got player firstname (%s) and lastname (%s)\n",playerName.GetData(), lastName.GetData() );

    ///////////////////////////////////////////////////////////////
    //  Check to see if the player name is valid
    ///////////////////////////////////////////////////////////////
    if ( playerName.Length() == 0 || !FilterName(playerName) )
    {
        psCharRejectedMessage reject(me->clientnum,
                                     psCharRejectedMessage::NON_LEGAL_NAME,
                                     "The name you specifed is not a legal player name." );

        psserver->GetEventManager()->SendMessage(reject.msg);
        return;
    }

    if ( lastName.Length() != 0 && !FilterName(lastName) )
    {
        psCharRejectedMessage reject(me->clientnum,
                                     psCharRejectedMessage::NON_LEGAL_NAME,
                                     "The name you specifed is not a legal lastname." );

        psserver->GetEventManager()->SendMessage(reject.msg);
        return;
    }

    Debug2( LOG_NEWCHAR, me->clientnum,"Checking player firstname '%s'..\n",playerName.GetData());
    ///////////////////////////////////////////////////////////////
    //  Check to see if the character name is unique in 'characters'.
    ///////////////////////////////////////////////////////////////
    if  ( !IsUnique( playerName ) )
    {
        psCharRejectedMessage reject(me->clientnum,
                                     psCharRejectedMessage::NON_UNIQUE_NAME,
                                     "The firstname you specifed is not unique." );

        psserver->GetEventManager()->SendMessage(reject.msg);
        return;
    }

    if (lastName.Length())
    {
        Debug2( LOG_NEWCHAR, me->clientnum,"Checking player lastname '%s'..\n",lastName.GetData());
        if  ( !IsLastNameUnique( lastName ) )
        {
            psCharRejectedMessage reject(me->clientnum,
                                        psCharRejectedMessage::NON_UNIQUE_NAME,
                                        "The lastname you specifed is not unique." );

            psserver->GetEventManager()->SendMessage(reject.msg);
            return;
        }
    }
    ///////////////////////////////////////////////////////////////
    //  Check to see if the character name is on the reserve list.
    ///////////////////////////////////////////////////////////////
    int reservedName = IsReserved( playerName, acctID );
    if ( reservedName == NAME_RESERVED )
    {
        csString error;
        error.Format("The name %s is reserved", playerName.GetData() );
        psCharRejectedMessage reject(me->clientnum,
                                     psCharRejectedMessage::RESERVED_NAME,
                                     (char*)error.GetData() );

        psserver->GetEventManager()->SendMessage(reject.msg);
        return;
    }


    csString error;
    if (  psserver->charCreationManager->Validate( upload, error) == false )
    {
        error.Append(", your creation choices are invalid." );
        psCharRejectedMessage reject(me->clientnum,
                                     psCharRejectedMessage::INVALID_CREATION,
                                     (char*)error.GetData() );

        reject.SendMessage();
        return;
    }        
    
    ///////////////////////////////////////////////////////////////
    //  Create the psCharacter structure for the player.
    ///////////////////////////////////////////////////////////////
    psCharacter *chardata=new psCharacter();
    chardata->SetCharType(PSCHARACTER_TYPE_PLAYER);
    chardata->SetFullName(playerName,lastName);
    chardata->SetDescription(upload.bio);

    psRaceInfo *raceinfo=CacheManager::GetSingleton().GetRaceInfoByNameGender( upload.race, (PSCHARACTER_GENDER)upload.gender);
    if (raceinfo==NULL)
    {
        Error3("Invalid race/gender combination on character creation:  Race='%d' Gender='%d'", upload.race, upload.gender );
        psCharRejectedMessage reject(me->clientnum);
        psserver->GetEventManager()->Broadcast(reject.msg, NetBase::BC_FINALPACKET);
        psserver->RemovePlayer (me->clientnum,"Player tried to create an invalid race/gender.");
        delete chardata;
        return;
    }
    chardata->SetRaceInfo(raceinfo);
    chardata->SetHitPoints(50.0);
    chardata->SetHitPointsMax(0.0);
    chardata->SetHitPointsMaxModifier(0.0);

    // Try tutorial level first.
    const char* sectorname = "tutorial";
    float x = -232.00f;
    float y = 21.31f;
    float z = 31.50f;
    float yrot = 4.0f;

    psSectorInfo *sectorinfo = CacheManager::GetSingleton().GetSectorInfoByName(sectorname);
    
    bool sectorFound = true;
    
    if ( sectorinfo && EntityManager::GetSingleton().FindSector(sectorinfo->name) == NULL )
    {
        Error2("Sector='%s' found but no map file was detected for it. Using NPCroom1", sectorname );    
        sectorinfo = CacheManager::GetSingleton().GetSectorInfoByName("NPCroom1");
        if ( sectorinfo && EntityManager::GetSingleton().FindSector(sectorinfo->name) == NULL )
        {
            Error1("NPCroom1 failed - Critical");                
            sectorFound = false;    
        }    
        else if ( sectorinfo && EntityManager::GetSingleton().FindSector(sectorinfo->name) )
        {
            sectorFound = true;
        }                        
        else
        {
            sectorFound = false;
        }
    }
    else if ( sectorinfo && EntityManager::GetSingleton().FindSector(sectorinfo->name) )
    {
        sectorFound = true;
    }
    else 
    {
        sectorFound = false;
    }
    
    
    if ( sectorFound == false )
    {
        Error2("Unresolvable starting sector='%s'", sectorname );
        psCharRejectedMessage reject(me->clientnum);
        psserver->GetEventManager()->Broadcast(reject.msg, NetBase::BC_FINALPACKET);
        psserver->RemovePlayer (me->clientnum,"No starting Sector.");
        delete chardata;
        return;    
    }
    
    

    uint32 newinstance = psserver->GetRandom(INT_MAX-1)+1;
    chardata->SetLocationInWorld(newinstance, sectorinfo, x, y, z, yrot);  // Chars will start on random instance

    psTrait * trait;
//    CPrintf(CON_DEBUG, "Trait: %d\n", upload.selectedFace );    
    trait = CacheManager::GetSingleton().GetTraitByID(upload.selectedFace);    
    if ( trait )
        chardata->SetTraitForLocation( trait->location, trait );
            
    trait = CacheManager::GetSingleton().GetTraitByID(upload.selectedHairStyle);    
    if ( trait )
        chardata->SetTraitForLocation( trait->location, trait );
    
    trait = CacheManager::GetSingleton().GetTraitByID(upload.selectedBeardStyle);    
    if ( trait )
        chardata->SetTraitForLocation( trait->location, trait );
    
    trait = CacheManager::GetSingleton().GetTraitByID(upload.selectedHairColour);    
    if ( trait )
        chardata->SetTraitForLocation( trait->location, trait );
    
    trait = CacheManager::GetSingleton().GetTraitByID(upload.selectedSkinColour);    
    if ( trait )
        chardata->SetTraitForLocation( trait->location, trait );
    
    printf("Uploaded Skin Colour: %d\n", upload.selectedSkinColour );
            
                        
    csString filename;
    filename.Format("/planeshift/models/%s/%s.cal3d",raceinfo->mesh_name,raceinfo->mesh_name);

    gemActor *actor = new gemActor( chardata,
                                    raceinfo->mesh_name,
                                    filename,
                                    newinstance,
                                    EntityManager::GetSingleton().FindSector(sectorinfo->name),
                                    csVector3(x,y,z),yrot,
                                    client->GetClientNum(),chardata->GetCharacterID());

    actor->SetupCharData();

    if( !upload.verify )
    {
        if (!psServer::CharacterLoader.NewCharacterData(acctID,chardata))
        {
            Error1("Character could not be created.");
            psCharRejectedMessage reject(me->clientnum);
            psserver->GetEventManager()->Broadcast(reject.msg, NetBase::BC_FINALPACKET);
            psserver->RemovePlayer (me->clientnum,"Your character could not be created in the database.");
            delete chardata;
            return;
        }
    }

    // Check to see if a path name was set. If so we will use that to generate 
    // the character starting stats and skills.    
    if ( upload.path != "None" )
    {
        // Progression Event name is PATH_PathName
        csString name("PATH_");
        name.Append(upload.path);
        ProgressionEvent* event = psserver->GetProgressionManager()->FindEvent( name.GetData() );
        if ( event )
        {
            // The script uses the race base character points to calculate starting stats.
            event->SetValue("CharPoints",  (double)raceinfo->initialCP );                                                                                                 
            psserver->GetProgressionManager()->ProcessEvent( event, actor, actor );
        }            
    }
    else
    {
    //int cpUsage = psserver->charCreationManager->CalculateCPChoice( upload.choices ) +
    //              psserver->charCreationManager->CalculateCPLife(upload.lifeEvents );    
    for ( size_t ci = 0; ci < upload.choices.GetSize(); ci++ )
    {
        psCharCreationManager::CreationChoice* choice = psserver->charCreationManager->FindChoice( upload.choices[ci] );
        if ( choice )
        {
            csString script (choice->eventScript.GetData() );
            csString name( psserver->charCreationManager->FindChoice( upload.choices[ci] )->name.GetData() );
            Debug3( LOG_NEWCHAR, me->clientnum,"Choice: %s Creation Script: %s", name.GetData(), script.GetData() );

            if ( choice->choiceArea == FATHER_JOB || choice->choiceArea == MOTHER_JOB )
            {
                ProgressionEvent* event = psserver->GetProgressionManager()->FindEvent( script.GetData() );
                if ( event )
                {
                    int modifier = (choice->choiceArea == FATHER_JOB) ? upload.fatherMod : upload.motherMod;
                    if ( modifier > 3 || modifier < 1 )
                    {
                        modifier = 1;
                    }
                    
                    event->SetValue("ParentStatus", (double)modifier );                                                               
                }
            }                           
            psserver->GetProgressionManager()->ProcessEvent( script, actor, actor );
        }            
        else
        {
            Debug2( LOG_NEWCHAR, me->clientnum,"Character Choice %d not found\n", upload.choices[ci] );
        }
    }
    for ( size_t li = 0; li < upload.lifeEvents.GetSize(); li++ )
    {
        csString script(psserver->charCreationManager->FindLifeEvent( upload.lifeEvents[li] )->eventScript.GetData() );
        Debug2( LOG_NEWCHAR, me->clientnum,"LifeEvent Script: %s", script.GetData() );

        psserver->GetProgressionManager()->ProcessEvent( script, actor, actor );
    }
    }
    
    if ( !upload.verify )
    {

        if ( reservedName == NAME_RESERVED_FOR_YOU )
        {
            AssignScript( chardata );
        }
        // This function recalculates the Max HP, Mana and Stamina of the new character
        chardata->RecalculateStats();

        // Make sure the new player have HP, Mana and Samina that was calculated
        chardata->SetHitPoints(chardata->GetHitPointsMax());
        chardata->SetMana(chardata->GetManaMax());
        chardata->SetStamina(chardata->GetStaminaMax(true),true);
        chardata->SetStamina(chardata->GetStaminaMax(false),false);
        
                   
        psServer::CharacterLoader.SaveCharacterData( chardata, actor );
        Debug1(LOG_NEWCHAR,me->clientnum,"Player Creation Complete");

        // Remove cached objects to make sure that the client gets a fresh character
        // list from the database if it logs out and in within 2 minutes.
        iCachedObject *obj = CacheManager::GetSingleton().RemoveFromCache(CacheManager::GetSingleton().MakeCacheName("list",client->GetAccountID()));
        if (obj)
        {
            obj->ProcessCacheTimeout();
            obj->DeleteSelf();
        }
        obj = CacheManager::GetSingleton().RemoveFromCache(CacheManager::GetSingleton().MakeCacheName("auth",client->GetAccountID()));
        if (obj)
        {
            obj->ProcessCacheTimeout();
            obj->DeleteSelf();
        }

        // Here everything is ok
        client->SetPlayerID(chardata->GetCharacterID());
        client->SetName(playerName);

        psCharApprovedMessage app(me->clientnum);
        if (app.valid)
            psserver->GetEventManager()->SendMessage(app.msg);
        else
            Bug2("Could not create valid psCharApprovedMessage for client %u.\n",me->clientnum);                               
    }    
    else
    {
        psCharVerificationMesg mesg( me->clientnum );
        int z;
        for ( z = 0; z < PSITEMSTATS_STAT_COUNT; z++ )
        {
            int value = chardata->GetAttributes()->GetStat( (PSITEMSTATS_STAT)z);
            if ( value > 0 )
            {
                mesg.AddStat( value, CacheManager::GetSingleton().Attribute2String((PSITEMSTATS_STAT)z));                
            }                                    
        }
        for ( z = 0; z < PSSKILL_COUNT; z++ )
        {
            int rank = chardata->GetSkills()->GetSkillRank( (PSSKILL)z, false );
            
            psSkillInfo* info = CacheManager::GetSingleton().GetSkillByID(z);
            csString name("Not found");
            if ( info )
                name.Replace( info->name );
            
            if ( rank > 0 )
            {
                mesg.AddSkill(rank, name);                
            }                                            
        }                
        mesg.Construct();
        mesg.SendMessage();
    }
    
    delete actor;

    if (!upload.verify)
    {
        // Remove char data from the cache
        iCachedObject *obj = CacheManager::GetSingleton().RemoveFromCache(CacheManager::GetSingleton().MakeCacheName("char", chardata->GetCharacterID()));
        if (obj)
        {
            obj->ProcessCacheTimeout();
            obj->DeleteSelf();
        }
    }
}

void psCharCreationManager::AssignScript( psCharacter* character )
{
/*
    csString query;
    csString escape;
    db->Escape( escape, character->GetCharName() );
    query.Format( "SELECT script FROM migration where username='%s' AND done !='Y'", escape.GetData());
    Result result (db->Select( query ) );

    if ( result.IsValid() && result.Count() == 1 )
    {
        csString script(result[0][0]);
        character->AppendProgressionScript( script );
        printf("Doing it: %s\n",character->progressionScript.GetData());
    }
    
    query.Format("UPDATE migration SET done='Y' WHERE username='%s'",escape.GetData());
    db->Command(query);
    */
}

int psCharCreationManager::IsReserved( const char* name, int acctID )
{
    // Check to see if this name is reserved.  Does this check by comparing
    // the email address of the account with that stored in the migration table.
    csString query;
    csString escape;
    db->Escape( escape, name );
    query.Format( "SELECT m.email FROM migration m WHERE m.username='%s'", escape.GetData() );
    Result result (db->Select( query ) );

    if ( result.IsValid() && result.Count() == 1 )
    {
        csString savedEmail( result[0][0] );

        query.Format( "SELECT username FROM accounts WHERE id=%d\n", acctID );

        Result result2(db->Select( query ) );
        if ( result2.IsValid() && result2.Count() == 1 )
        {
            csString email( result2[0][0] );
            if ( savedEmail.CompareNoCase(email) )
                return NAME_RESERVED_FOR_YOU;
            else
                return NAME_RESERVED;
        }
    }

    return NAME_AVAILABLE;
}

bool psCharCreationManager::IsUnique( const char* name )
{
    csString escape;
    db->Escape( escape, name );
    // Check to see if name already exists in character database.
    csString query;
    query.Format( "Select id from characters where name='%s'", escape.GetData() );
    Result result (db->Select( query ) );
    return ! ( result.IsValid() && result.Count() >= 1 );
}

bool psCharCreationManager::IsLastNameUnique( const char* lastname )
{
    if (lastname && strlen(lastname))
    {
        // Check to see if name already exists in character database.
        csString query;
        csString escape;
        db->Escape(escape, lastname);
        query.Format( "Select id from characters where lastname='%s'", escape.GetData() );
        Result result (db->Select( query ) );
        if ( result.IsValid() && result.Count() >= 1 )
           return false;
   }
   return true;  // blank last name is now allowed.
}

bool psCharCreationManager::FilterName(const char* name)
{
    if (name == NULL)
        return false;

    size_t len = strlen(name);

    if ( (strspn(name,"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ")
                != len) )
        return false;

    if (((int)strspn(((const char*)name)+1,
                     (const char*)"abcdefghijklmnopqrstuvwxyz") != (int)len - 1))
        return false;

    return true;
}


bool psCharCreationManager::HandleCharCreateCP( MsgEntry* me, Client*client ) 
{
    int32_t raceID =  me->GetInt32();
    int32_t raceCPValue = 0;
    if (raceID>=0  &&  raceID<raceCPValuesLength)
    {
        raceCPValue = (int32_t)raceCPValues[raceID].value;
    }
    else
    {
        Error2("Invalid raceID %i", raceID);
    }

    psCharCreateCPMessage response(me->clientnum,raceID, raceCPValue);
    psserver->GetEventManager()->SendMessage(response.msg);

    return true;
}
