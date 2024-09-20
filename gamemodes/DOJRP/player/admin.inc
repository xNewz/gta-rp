#include <YSI_Coding\y_hooks>

enum staff_data
{
	Float:staff_lastx,
	Float:staff_lasty,
	Float:staff_lastz,
	Float:staff_lasta,
	Float:staff_lasthealth,
	Float:staff_lastarmour,
	staff_lastinterior,
	staff_lastworld,
	staff_specid,
	staff_giveid,
	staff_giveitem,
	bool:staff_spectating
};
new StaffData[MAX_PLAYERS][staff_data];

enum temp_data
{
	temp_vehicle_id
};
new TempData[MAX_PLAYERS][temp_data];

enum elements_data
{
	server_time
};
new ElementsData[elements_data];

stock ProcessServerElements(bool:rollover)
{
	if(rollover)
	{
		ElementsData[server_time]++;
    	if(ElementsData[server_time] >= 23)
    	{
    		ElementsData[server_time] = 0;
    	}
	}
	else 
	{
		ElementsData[server_time] = random(24);
	}
	SetWorldTime(ElementsData[server_time]);

	new weather = random(20);
	return SetWeather(weather);
}

stock CheckIfAdminOnline(masterid)
{
	foreach(new i : Player)
	{
		if(GetPlayerMasterID(i) == masterid) return i;
	}
	return INVALID_PLAYER_ID;
}

stock ShowAdminDialog(playerid)
{
	return ShowPlayerDialog(playerid, ADMIN_DIALOG, DIALOG_STYLE_MSGBOX,
"{FFFFFF}Admin Commands", "{FFFFFF}/(a)nn /bcmds /duty /kick /(un)ban /warn /(un)mute /(un)spec /get /goto /goback /disarm /emptyinv /time /weather\n/move /cam /tele /a(bike/heli/boat) /set(level/score/skin/money) /(give/remove)vip /give(item/weapon)", "Close", "");
}

stock ShowBuilderDialog(playerid)
{
	return ShowPlayerDialog(playerid, BUILDER_DIALOG, DIALOG_STYLE_MSGBOX,
"{FFFFFF}Building Commands", "{FFFFFF}/create(stop/sprunk/house/business) /delete(stop/sprunk/house/business) /deleteall(houses/businesses)\n/reset(house/business)price /resetall(house/business)prices /remove(house/business)owner /removeall(house/business)owners", "Close", "");
}

stock ShowTeleportDialog(playerid)
{
	return ShowPlayerDialog(playerid, TELE_MAIN_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}Teleport > Menu", "{FFFFFF}LS\nSF\nLV", "Select", "Cancel");
}

stock ReturnPlayerLevel(playerid)
{
	new level[16];
	if(GetPlayerLevel(playerid) == HELPER) { level = "Helper"; }
	else if(GetPlayerLevel(playerid) == ADMIN) { level = "Admin"; }
	else if(GetPlayerLevel(playerid) == HEAD) { level = "Head Admin"; }
	else if(GetPlayerLevel(playerid) == MANAGER) { level = "Manager"; }
	else if(GetPlayerLevel(playerid) == OWNER) { level = "Owner"; }
	else
	{
		if(GetPlayerVIP(playerid))
		{
			level = "V.I.P";
		}
		else
		{
			level = "Player";
		}
	}
	return level;
}

stock ReturnLevelName(level)
{
	new name[16];
	if(level == HELPER) { name = "Helper"; }
	else if(level == ADMIN) { name = "Admin"; }
	else if(level == HEAD) { name = "Head Admin"; }
	else if(level == MANAGER) { name = "Manager"; }
	else if(level == OWNER) { name = "Owner"; }
	else
	{
		name = "Player";
	}
	return name;
}

stock StaffRadio(colour, text[])
{
	new string[144];
	foreach(new i : Player)
	{
	    if(GetPlayerLevel(i) > PLAYER)
	    {
	    	format(string, sizeof(string), "%s", text);
	    	SendClientMessage(i, colour, string);
	    }
	}
	return 1;
}

stock OnPlayerSpectate(playerid, targetid)
{
	new Float:pos[4], Float:health, Float:armour;
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot spectate yourself.");
    if(IsPlayerSafe(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot spectate players while you are in a safezone.");
    if(GetDutyStatus(playerid) == true) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot spectate players while you are on duty.");
    if(GetPlayerJailTime(playerid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot spectate players while you are in jail.");
    if(StaffData[targetid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: That player is currently in spectate mode.");

    if(StaffData[playerid][staff_spectating] == false)
    {
        GetPlayerHealth(playerid, health);
        SetStoredHealth(playerid, health);

        GetPlayerArmour(playerid, armour);
        SetStoredArmour(playerid, armour);

        StoreWeaponData(playerid);
        ResetPlayerWeapons(playerid);

	    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	    GetPlayerFacingAngle(playerid, pos[3]);

	    StaffData[playerid][staff_lastx] = pos[0];
	    StaffData[playerid][staff_lasty] = pos[1];
	    StaffData[playerid][staff_lastz] = pos[2];
	    StaffData[playerid][staff_lasta] = pos[3];
	    StaffData[playerid][staff_lastinterior] = GetPlayerInterior(playerid);
	    StaffData[playerid][staff_lastworld] = GetPlayerVirtualWorld(playerid);

		SetStoredWantedLevel(playerid, GetPlayerWantedLevel(playerid));
		SetStoredSkin(playerid, GetPlayerSkin(playerid));
    }
    StaffData[playerid][staff_spectating] = true;
  	StaffData[playerid][staff_specid] = targetid;

	SetPlayerSpectated(targetid, true);

	TogglePlayerHUDTextdraws(playerid, false);

	TogglePlayerSpectating(playerid, true);
	if(IsPlayerInAnyVehicle(targetid))
	{
		SetPlayerInterior(playerid, GetPlayerInterior(targetid));
	  	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
	  	return PlayerSpectateVehicle(playerid, GetPlayerVehicleID(targetid));
	}
	else
	{
		SetPlayerInterior(playerid, GetPlayerInterior(targetid));
		SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
	  	PlayerSpectatePlayer(playerid, targetid);
    }
    return 1;
}

hook OnGameModeInit()
{ 
	ElementsData[server_time] = 12;
	return ProcessServerElements(false);
}

hook OnPlayerSpawn(playerid)
{
	if(StaffData[playerid][staff_spectating] == true)
	{
		SetPlayerHealth(playerid, GetStoredHealth(playerid));
        SetPlayerArmour(playerid, GetStoredArmour(playerid));

		RestoreStoredWeapons(playerid);

	    SetPlayerInterior(playerid, StaffData[playerid][staff_lastinterior]);
	    SetPlayerVirtualWorld(playerid, StaffData[playerid][staff_lastworld]);
	    SetPlayerPos(playerid, StaffData[playerid][staff_lastx], StaffData[playerid][staff_lasty], StaffData[playerid][staff_lastz]);
	    SetPlayerFacingAngle(playerid, StaffData[playerid][staff_lasta]);

		SetPlayerWantedLevel(playerid, GetStoredWantedLevel(playerid));

		SetPlayerSkin(playerid, GetStoredSkin(playerid));

		TogglePlayerHUDTextdraws(playerid, true);
		
		SetPlayerSpectated(StaffData[playerid][staff_specid], false);
		StaffData[playerid][staff_specid] = INVALID_PLAYER_ID;
		StaffData[playerid][staff_spectating] = false;
	}
	return 1;
}

hook OnPlayerConnect(playerid)
{
	if(!IsPlayerNPC(playerid))
	{
		StaffData[playerid][staff_lastx] = 0.0;
		StaffData[playerid][staff_lasty] = 0.0;
		StaffData[playerid][staff_lastz] = 0.0;
		StaffData[playerid][staff_lasta] = 0.0;

		StaffData[playerid][staff_lastinterior] = 0;
		StaffData[playerid][staff_lastworld] = 0;

		StaffData[playerid][staff_specid] = 0;

		StaffData[playerid][staff_giveid] = INVALID_PLAYER_ID;
		StaffData[playerid][staff_giveitem] = 0;

		StaffData[playerid][staff_spectating] = false;

		TempData[playerid][temp_vehicle_id] = INVALID_VEHICLE_ID;
	}
	return 1;
}

hook OnPlayerDisconnect(playerid, reason)
{
	if(!IsPlayerNPC(playerid))
	{
		if(TempData[playerid][temp_vehicle_id] != INVALID_VEHICLE_ID)
		{
			DestroyVehicle(TempData[playerid][temp_vehicle_id]);
			TempData[playerid][temp_vehicle_id] = INVALID_VEHICLE_ID;
		}
	}
	return 1;
}

hook OnPlayerClickPlayer(playerid, clickedplayerid, source)
{
	if(GetPlayerLevel(playerid) >= ADMIN && !IsPlayerInjured(playerid))
	{
		if(!IsPlayerNPC(clickedplayerid))
		{
			OnPlayerSpectate(playerid, clickedplayerid);
		}
	}
	return 1;
}

hook OnPlayerInteriorChange(playerid, newinteriorid, oldinteriorid)
{
	if(GetPlayerSpectated(playerid) == true)
	{
	    if(newinteriorid != oldinteriorid)
	    {
		    foreach(new i : Player)
			{
				if(StaffData[i][staff_spectating] == true && StaffData[i][staff_specid] == playerid)
				{
					SetPlayerInterior(i, GetPlayerInterior(playerid));
					SetPlayerVirtualWorld(i, GetPlayerVirtualWorld(playerid));
					return 1;
				}
			}
		}
	}
	return 1;
}

hook OnPlayerStateChange(playerid, newstate, oldstate)
{
	if(oldstate == PLAYER_STATE_DRIVER && newstate == PLAYER_STATE_ONFOOT && TempData[playerid][temp_vehicle_id] != INVALID_VEHICLE_ID)
	{
		DestroyVehicle(TempData[playerid][temp_vehicle_id]);
		TempData[playerid][temp_vehicle_id] = INVALID_VEHICLE_ID;
		return 1;
	}
	
	if(GetPlayerSpectated(playerid) == true)
	{
	    if(newstate != oldstate)
	    {
		    foreach(new i : Player)
			{
				if(StaffData[i][staff_spectating] == true && StaffData[i][staff_specid] == playerid)
				{
					if(IsPlayerInAnyVehicle(playerid))
					{
						PlayerSpectateVehicle(i, GetPlayerVehicleID(playerid));
						return 1;
					}
					else
					{
						PlayerSpectatePlayer(i, playerid);
						return 1;
					}
				}
			}
		}
	}
	return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
		case TELE_LV_DIALOG:
		{
			if(response)
			{
				switch(listitem)
				{
					case 0://LVPD
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 2336.8965, 2456.7803, 14.9688);
						SetPlayerFacingAngle(playerid, 176.4752);
						return SetCameraBehindPlayer(playerid);
					}
					case 1://Airport
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 1674.5664, 1447.8522, 10.7900);
						SetPlayerFacingAngle(playerid, 269.8492);
						return SetCameraBehindPlayer(playerid);
					}
					case 2://Hospital
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 1607.4288, 1817.2189, 10.8203);
						SetPlayerFacingAngle(playerid, 359.7627);
						return SetCameraBehindPlayer(playerid);
					}
					case 3://University
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 2375.5913, 2308.0605, 8.1406);
						SetPlayerFacingAngle(playerid, 359.8412);
						return SetCameraBehindPlayer(playerid);
					}
					case 4://Binco
					{
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerInterior(playerid, 0);
						SetPlayerPos(playerid, 1655.6273, 1733.3518, 10.8281);
						SetPlayerFacingAngle(playerid, 91.1724);
						return SetCameraBehindPlayer(playerid);
					}
				}
				return 1;
			}
			else
			{
				ShowTeleportDialog(playerid);
			}
			return 1;
		}
		case TELE_SF_DIALOG:
		{
			if(response)
			{
				switch(listitem)
				{
					case 0://SFPD
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, -1605.5231, 712.1763, 13.8672);
						SetPlayerFacingAngle(playerid, 354.2668);
						return SetCameraBehindPlayer(playerid);
					}
					case 1://Airport
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, -1422.7832, -288.7293, 14.1484);
						SetPlayerFacingAngle(playerid, 136.3390);
						return SetCameraBehindPlayer(playerid);
					}
					case 2://Hospital
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, -2655.1104, 638.3422, 14.4531);
						SetPlayerFacingAngle(playerid, 180.1129);
						return SetCameraBehindPlayer(playerid);
					}
					case 3://University
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, -1493.7815, 920.1070, 7.1875);
						SetPlayerFacingAngle(playerid, 90.0846);
						return SetCameraBehindPlayer(playerid);
					}
					case 4://Binco
					{
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerInterior(playerid, 0);
						SetPlayerPos(playerid, -2375.7122, 910.1544, 45.4453);
						SetPlayerFacingAngle(playerid, 89.9471);
						return SetCameraBehindPlayer(playerid);
					}
				}
				return 1;
			}
			else
			{
				ShowTeleportDialog(playerid);
			}
			return 1;
		}
		case TELE_LS_DIALOG:
		{
			if(response)
			{
				switch(listitem)
				{
					case 0://LSPD
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 1553.8295, -1675.6392, 16.1953);
						SetPlayerFacingAngle(playerid, 89.2095);
						return SetCameraBehindPlayer(playerid);
					}
					case 1://Airport
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 1642.2411, -2332.7742, 13.5469);
						SetPlayerFacingAngle(playerid, 0.1502);
						return SetCameraBehindPlayer(playerid);
					}
					case 2://Hospital
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 2025.2469, -1403.4918, 17.2098);
						SetPlayerFacingAngle(playerid, 180.7238);
						return SetCameraBehindPlayer(playerid);
					}
					case 3://University
					{
						SetPlayerInterior(playerid, 0);
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerPos(playerid, 1467.2743, -1011.5594, 26.8438);
						SetPlayerFacingAngle(playerid, 179.5822);
						return SetCameraBehindPlayer(playerid);
					}
					case 4://Binco
					{
						SetPlayerVirtualWorld(playerid, 0);
						SetPlayerInterior(playerid, 0);
						SetPlayerPos(playerid, 2244.6360, -1664.5170, 15.4766);
						SetPlayerFacingAngle(playerid, 346.0298);
						return SetCameraBehindPlayer(playerid);
					}
				}
				return 1;
			}
			else
			{
				ShowTeleportDialog(playerid);
			}
			return 1;
		}
		case TELE_MAIN_DIALOG:
		{
			if(response)
			{
				switch(listitem)
				{
					case 0://LS
					{
						ShowPlayerDialog(playerid, TELE_LS_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}Teleport > LS", "{FFFFFF}LSPD\nAirport\nHospital\nUniversity\nBinco", "Teleport", "Back");
					}
					case 1://SF
					{
						ShowPlayerDialog(playerid, TELE_SF_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}Teleport > SF", "{FFFFFF}SFPD\nAirport\nHospital\nUniversity\nBinco", "Teleport", "Back");
					}
					case 2://LV
					{
						ShowPlayerDialog(playerid, TELE_LV_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}Teleport > LV", "{FFFFFF}LVPD\nAirport\nHospital\nUniversity\nBinco", "Teleport", "Back");
					}
				}
			}
			return 1;
		}
		case GIVE_ITEM_DIALOG:
		{
			if(response)
			{
				switch(listitem)
				{
					case 0://Sprunk
					{
						StaffData[playerid][staff_giveitem] = SPRUNK_MODEL;
					}
					case 1://Alcohol
					{
						StaffData[playerid][staff_giveitem] = ALCOHOL_MODEL;
					}
					case 2://Burger
					{
						StaffData[playerid][staff_giveitem] = BURGER_MODEL;
					}
					case 3://Taco
					{
						StaffData[playerid][staff_giveitem] = TACO_MODEL;
					}
					case 4://Pizza
					{
						StaffData[playerid][staff_giveitem] = PIZZA_MODEL;
					}
					case 5://Medikit
					{
						StaffData[playerid][staff_giveitem] = MEDIKIT_MODEL;
					}
					case 6://Toolbox
					{
						StaffData[playerid][staff_giveitem] = TOOLBOX_MODEL;
					}
					case 7://Gun Materials
					{
						StaffData[playerid][staff_giveitem] = MATERIALS_MODEL;
					}
					case 8://Jerrycan
					{
						StaffData[playerid][staff_giveitem] = JERRYCAN_MODEL;
					}
					case 9://Crowbar
					{
						StaffData[playerid][staff_giveitem] = CROWBAR_MODEL;
					}
					case 10://Lotto Ticket
					{
						StaffData[playerid][staff_giveitem] = TICKET_MODEL;
					}
					case 11://Seeds
					{
						StaffData[playerid][staff_giveitem] = SEEDS_MODEL;
					}
					case 12://Weed
					{
						StaffData[playerid][staff_giveitem] = WEED_MODEL;
					}
					case 13://Cell Phone
					{
						StaffData[playerid][staff_giveitem] = PHONE_MODEL;
					}
				}

				new string[200];
				format(string, sizeof(string), "{FFFFFF}How much (%s) would you like to give them?:", ReturnItemName(StaffData[playerid][staff_giveitem]));
				ShowPlayerDialog(playerid, GIVE_AMOUNT_DIALOG, DIALOG_STYLE_INPUT, "{FFFFFF}Give Item", string, "Give", "Cancel");
			}
			return 1;
		}
		case GIVE_AMOUNT_DIALOG:
		{
			if(response)
			{
				new string[300], admin[24], player[24], targetid = StaffData[playerid][staff_giveid], item = StaffData[playerid][staff_giveitem], amount = strval(inputtext);
				if(!IsNumeric(inputtext) || amount < 1) return SendClientMessage(playerid, RED, "ล้มเหลว: You must input a number greater that 0.");
				if(amount > 10) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot give them more than x10 items at a time.");
				if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: That player is not connected anymore.");
				if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: That player is not spawned anymore.");

				if(GetPlayerVIP(targetid))
				{
					if((ReturnInventoryWeight(targetid) + (ReturnItemWeight(item) * amount)) > MAX_VIP_WEIGHT) return SendClientMessage(playerid, RED, "ล้มเหลว: The item(s) are too heavy to fit in the player's inventory.");
				}
				else
				{
					if((ReturnInventoryWeight(targetid) + (ReturnItemWeight(item) * amount)) > MAX_PLAYER_WEIGHT) return SendClientMessage(playerid, RED, "ล้มเหลว: The item(s) are too heavy to fit in the player's inventory.");
				}

				new slot = CheckInventoryForItem(targetid, item);
				if(slot == 0)
				{
					slot = GetFreeInventorySlot(targetid);
					if(slot == 0) return SendClientMessage(playerid, RED, "ล้มเหลว: The player doesn't have any free inventory slots.");
				}

				if(item == TICKET_MODEL)
				{
					if(CheckInventoryForItem(targetid, TICKET_MODEL) != 0) return SendClientMessage(playerid, RED, "ล้มเหลว: Players can only hold x1 Lotto Ticket at a time.");
					if(amount > 1) return SendClientMessage(playerid, RED, "ล้มเหลว: You cannot give players more than x1 Lotto Ticket.");
					ProcessLottoTicket(targetid);
				}
				else if(item == PHONE_MODEL)
				{
					if(!GetPlayerPhone(targetid))
					{
						GivePlayerPhone(targetid);
					}
				}

				AddItemToInventory(targetid, item, amount, false);

				GetPlayerName(playerid, admin, sizeof(admin));
				GetPlayerName(targetid, player, sizeof(player));

				if(targetid != playerid)
				{
					format(string, sizeof(string), "แอดมิน: [%s] %s (%i) has given you x%i %s.", ReturnPlayerLevel(playerid), admin, playerid, amount, ReturnItemName(item));
					SendClientMessage(targetid, YELLOW, string);
				}

				format(string, sizeof(string), "แอดมิน: You have given %s (%i) x%i %s.", player, targetid, amount, ReturnItemName(item));
				SendClientMessage(playerid, YELLOW, string);
				return 1;
			}
		}
	}
	return 1;
}

task OnElementsUpdate[300000]()
{
	ProcessServerElements(true);
	return 1;
}

YCMD:acmds(playerid, params[], help)
{
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	return ShowAdminDialog(playerid);
}

YCMD:bcmds(playerid, params[], help)
{
	if(GetPlayerLevel(playerid) < OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับเจ้าของเซิร์ฟเวอร์เท่านั้น.");
	return ShowBuilderDialog(playerid);
}

YCMD:a(playerid, params[], help)
{
	new message[100], string[144], name[24];
    if(GetPlayerLevel(playerid) < HELPER) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ช่วยขึ้นไปเท่านั้น.");
    if(sscanf(params, "s[100]", message)) return SendClientMessage(playerid, RED, "วิธีใช้: /a [ข้อความ]");
	if(strlen(message) < 1 || strlen(message) > 100) return SendClientMessage(playerid, RED, "ล้มเหลว: ข้อความของคุณต้องมีความยาวระหว่าง 1-100 ตัวอักษร.");
	GetPlayerName(playerid, name, sizeof(name));
	format(string, sizeof(string), "[%s] %s: %s", ReturnPlayerLevel(playerid), name, message);
    StaffRadio(PINK, string);
	return 1;
}

YCMD:ann(playerid, params[], help)
{
    new message[64], string[128];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "s[64]", message)) return SendClientMessage(playerid, RED, "วิธีใช้: /ann [ข้อความ]");
	if(strlen(message) < 2 || strlen(message) > 64) return SendClientMessage(playerid, RED, "ล้มเหลว: ข้อความของคุณต้องมีความยาวระหว่าง 2-64 ตัวอักษร.");
	format(string, sizeof(string), "~w~%s", message);
	GameTextForAll(string, 7000, 6);
	return 1;
}

YCMD:tele(playerid, params[], help)
{
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");
	return ShowTeleportDialog(playerid);
}

YCMD:abike(playerid, params[], help)
{
	new Float:pos[4];
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่บนพื้นเท้าเพื่อใช้คำสั่งนี้.");
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ในภายใน interiors.");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	GetPlayerFacingAngle(playerid, pos[3]);

	TempData[playerid][temp_vehicle_id] = CreateVehicle(522, pos[0], pos[1], pos[2], pos[3], random(255), random(255), -1, 0);
	return PutPlayerInVehicle(playerid, TempData[playerid][temp_vehicle_id], 0);
}

YCMD:aheli(playerid, params[], help)
{
	new Float:pos[4];
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่บนพื้นเท้าเพื่อใช้คำสั่งนี้.");
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ในภายใน interiors.");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	GetPlayerFacingAngle(playerid, pos[3]);

	TempData[playerid][temp_vehicle_id] = CreateVehicle(469, pos[0], pos[1], pos[2], pos[3], random(255), random(255), -1, 0);
	return PutPlayerInVehicle(playerid, TempData[playerid][temp_vehicle_id], 0);
}

YCMD:aboat(playerid, params[], help)
{
	new Float:pos[4];
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(GetPlayerState(playerid) != PLAYER_STATE_ONFOOT) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่บนพื้นเท้าเพื่อใช้คำสั่งนี้.");
	if(GetPlayerInterior(playerid) != 0 || GetPlayerVirtualWorld(playerid) != 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ในภายใน interiors.");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	GetPlayerFacingAngle(playerid, pos[3]);

	TempData[playerid][temp_vehicle_id] = CreateVehicle(452, pos[0], pos[1], pos[2], pos[3], random(255), random(255), -1, 0);
	return PutPlayerInVehicle(playerid, TempData[playerid][temp_vehicle_id], 0);
}

YCMD:mute(playerid, params[], help)
{
	new targetid, reason[32], string[128], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "us[32]", targetid, reason)) return SendClientMessage(playerid, RED, "วิธีใช้: /mute [ชื่อผู้เล่น] [เหตุผล]");
	if(strlen(reason) < 2 || strlen(reason) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: เหตุผลของคุณต้องมีความยาวระหว่าง 2-32 ตัวอักษร.");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(GetPlayerLevel(targetid) > PLAYER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถปิดเสียง staff ด้วยกันเองได้.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถปิดเสียงตัวเองได้.");
    if(GetPlayerMuted(targetid) == true) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นถูกปิดเสียงไว้แล้ว.");

    SetPlayerMuted(targetid, true);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ปิดเสียงคุณเนื่องจาก: %s", ReturnPlayerLevel(playerid), admin, playerid, reason);
	SendClientMessage(targetid, ORANGE, string);

	format(string, sizeof(string), "แอดมิน: ได้ปิดเสียง %s (%i) แล้ว.", player, targetid);
	SendClientMessage(playerid, ORANGE, string);
	return 1;
}

CMD:unmute(playerid, params[])
{
	new targetid, string[128], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /unmute [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
    if(GetPlayerMuted(targetid) == false) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ถูกปิดเสียงไว้.");

	SetPlayerMuted(targetid, false);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ยกเลิกการปิดเสียงคุณแล้ว.", ReturnPlayerLevel(playerid), admin, playerid);
	SendClientMessage(targetid, YELLOW, string);

	format(string, sizeof(string), "แอดมิน: ได้ยกเลิกการปิดเสียง %s (%i) แล้ว.", player, targetid);
	SendClientMessage(playerid, YELLOW, string);
	return 1;
}

YCMD:warn(playerid, params[], help)
{
	new targetid, string[144], reason[32], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "us[32]", targetid, reason)) return SendClientMessage(playerid, RED, "วิธีใช้: /warn [ชื่อผู้เล่น] [เหตุผล]");
	if(strlen(reason) < 2 || strlen(reason) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: เหตุผลของคุณต้องมีความยาวระหว่าง 2-32 ตัวอักษร.");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(GetPlayerLevel(targetid) > PLAYER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถเตือน staff ด้วยกันเองได้.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถเตือนตัวเองได้.");
	
	SetPlayerWarns(targetid, GetPlayerWarns(targetid) + 1);
	
	if(GetPlayerWarns(targetid) >= MAX_WARNS)
	{
	    GetPlayerName(playerid, admin, sizeof(admin));
		GetPlayerName(targetid, player, sizeof(player));

	    format(string, sizeof(string), "ระบบ: %s (%i) ได้ถูกเตือนเกิน %i/%i ครั้งและถูกเตะออกโดยอัตโนมัติเนื่องจาก: %i/%i Warnings", player, targetid, MAX_WARNS, MAX_WARNS);
	    SendClientMessageToAll(RED, string);
		return KickPlayer(targetid);
	}
	else
	{
	    format(string, sizeof(string), "เตือน: [%s] %s (%i) ได้เตือนคุณเนื่องจาก: %s (%i/%i)", ReturnPlayerLevel(playerid), admin, playerid, reason, GetPlayerWarns(targetid), MAX_WARNS);
	    SendClientMessageToAll(RED, string);
	}
    return 1;
}

YCMD:emptyinv(playerid, params[], help)
{
	new targetid, string[128], empty[32], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /emptyinv [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	format(empty, sizeof(empty), "Empty Slot");

    EmptyPlayerInventory(targetid);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ล้างคลังของคุณ.", ReturnPlayerLevel(playerid), admin, playerid);
		SendClientMessage(targetid, ORANGE, string);
	}
	format(string, sizeof(string), "แอดมิน: คุณได้ล้างคลังของ %s (%i) แล้ว.", player, targetid);
	SendClientMessage(playerid, ORANGE, string);
	return 1;
}

YCMD:disarm(playerid, params[], help)
{
	new targetid, string[128], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /disarm [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

    ResetPlayerWeapons(targetid);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ล้างอาวุธของคุณ.", ReturnPlayerLevel(playerid), admin, playerid);
		SendClientMessage(targetid, ORANGE, string);
	}
	format(string, sizeof(string), "แอดมิน: คุณได้ล้างอาวุธของ %s (%i) แล้ว.", player, targetid);
	SendClientMessage(playerid, ORANGE, string);
	return 1;
}

YCMD:setmoney(playerid, params[], help)
{
	new string[128], admin[24], player[24], targetid, money;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "ud", targetid, money)) return SendClientMessage(playerid, RED, "วิธีใช้: /setmoney [ชื่อผู้เล่น] [จำนวน]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(money < 0) return SendClientMessage(playerid, RED, "ล้มเหลว: จำนวนเงินต้องมากกว่า 0.");

	ResetPlayerMoney(targetid);
	GivePlayerMoney(targetid, money);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ตั้งค่าเงินของคุณเป็น: $%i", ReturnPlayerLevel(playerid), admin, playerid, money);
		SendClientMessage(targetid, YELLOW, string);
	}
	format(string, sizeof(string), "แอดมิน: คุณได้ตั้งค่าเงินของ %s (%i) เป็น: $%i", player, targetid, money);
	SendClientMessage(playerid, YELLOW, string);
    return 1;
}

YCMD:setskin(playerid, params[], help)
{
	new string[128], admin[24], player[24], targetid, skinid;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "ud", targetid, skinid)) return SendClientMessage(playerid, RED, "วิธีใช้: /setskin [ชื่อผู้เล่น] [สกิน ID]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(skinid < 1 || skinid > 299 || skinid == 74) return SendClientMessage(playerid, RED, "ล้มเหลว: สกิน ID ต้องอยู่ระหว่าง 1-299 และไม่ใช่ 74.");

	SetPlayerSkin(targetid, skinid);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ตั้งค่าสกินของคุณเป็น: %i", ReturnPlayerLevel(playerid), admin, playerid, skinid);
		SendClientMessage(targetid, YELLOW, string);
	}
	format(string, sizeof(string), "แอดมิน: ได้ตั้งค่าสกินของ %s (%i) เป็น: %i", player, targetid, skinid);
	SendClientMessage(playerid, YELLOW, string);
    return 1;
}

YCMD:setscore(playerid, params[], help)
{
	new string[128], admin[24], player[24], targetid, score;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "ud", targetid, score)) return SendClientMessage(playerid, RED, "วิธีใช้: /setscore [ชื่อผู้เล่น] [คะแนน]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	SetPlayerScore(targetid, score);

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ตั้งค่าคะแนนของคุณเป็น: %i", ReturnPlayerLevel(playerid), admin, playerid, score);
		SendClientMessage(targetid, YELLOW, string);
	}

	format(string, sizeof(string), "แอดมิน: ได้ตั้งค่าคะแนนของ %s (%i) เป็น: %i", player, targetid, score);
	SendClientMessage(playerid, YELLOW, string);
    return 1;
}

YCMD:goback(playerid, params[], help)
{
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(StaffData[playerid][staff_lastx] == 0.0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ได้เคยใช้คำสั่ง /goto ก่อนหน้านี้.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");

	SetPlayerInterior(playerid, StaffData[playerid][staff_lastinterior]);
    SetPlayerVirtualWorld(playerid, StaffData[playerid][staff_lastworld]);

    SetPlayerPos(playerid, StaffData[playerid][staff_lastx], StaffData[playerid][staff_lasty], StaffData[playerid][staff_lastz]);
    SetPlayerFacingAngle(playerid, StaffData[playerid][staff_lasta]);

    StaffData[playerid][staff_lastx] = 0.0;
	StaffData[playerid][staff_lasty] = 0.0;
	StaffData[playerid][staff_lastz] = 0.0;
	StaffData[playerid][staff_lasta] = 0.0;

	StaffData[playerid][staff_lastinterior] = 0;
	StaffData[playerid][staff_lastworld] = 0;
	return 1;
}

YCMD:goto(playerid, params[], help)
{
	new targetid, Float:pos[3], Float:get[4];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /goto [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถไปหาตัวเองได้.");
    if(GetPlayerJailTime(targetid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นถูกจับไว้ในเรือนจำ.");
    if(GetPlayerJailTime(playerid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณถูกจับไว้ในเรือนจำ.");
    if(StaffData[playerid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้เมื่อคุณกำลังดูผู้เล่น.");
    if(StaffData[targetid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นกำลังถูกดูโดยผู้ดูแลระบบ.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");

    GetPlayerPos(playerid, get[0], get[1], get[2]);
    GetPlayerFacingAngle(playerid, get[3]);

	StaffData[playerid][staff_lastx] = get[0];
	StaffData[playerid][staff_lasty] = get[1];
	StaffData[playerid][staff_lastz] = get[2];
	StaffData[playerid][staff_lasta] = get[3];

	StaffData[playerid][staff_lastinterior] = GetPlayerInterior(playerid);
	StaffData[playerid][staff_lastworld] = GetPlayerVirtualWorld(playerid);

    GetPlayerPos(targetid, pos[0], pos[1], pos[2]);

	if(IsPlayerInAnyVehicle(playerid))
    {
        if(GetPlayerInterior(targetid) == 0 && GetPlayerVirtualWorld(targetid) == 0)
        {
			SetVehiclePos(GetPlayerVehicleID(playerid), pos[0], pos[1], pos[2]);
		}
		else
		{
    		SetPlayerInterior(playerid, GetPlayerInterior(targetid));
    		SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
    		SetPlayerPos(playerid, pos[0], pos[1], pos[2]);
    	}
    	return 1;
	}
	else
	{
    	SetPlayerInterior(playerid, GetPlayerInterior(targetid));
    	SetPlayerVirtualWorld(playerid, GetPlayerVirtualWorld(targetid));
    	SetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	}
	return 1;
}

YCMD:get(playerid, params[], help)
{
	new targetid, Float:pos[3];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /get [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถไปหาตัวเองได้.");
    if(GetPlayerJailTime(targetid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นถูกจับไว้ในเรือนจำ.");
    if(GetPlayerJailTime(playerid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณถูกจับไว้ในเรือนจำ.");
    if(StaffData[playerid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้เมื่อคุณกำลังดูผู้เล่น.");
    if(StaffData[targetid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นกำลังถูกดูโดยผู้ดูแลระบบ.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");

    GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

	if(IsPlayerInAnyVehicle(targetid))
    {
        if(GetPlayerInterior(playerid) == 0 && GetPlayerVirtualWorld(playerid) == 0)
		{
	    	SetVehiclePos(GetPlayerVehicleID(targetid), pos[0], pos[1], pos[2]);
	    }
	    else
	    {
	    	SetPlayerInterior(targetid, GetPlayerInterior(playerid));
	    	SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(playerid));
	    	SetPlayerPos(targetid, pos[0], pos[1], pos[2]);
	    }
	    return 1;
    }
    else
    {
	    SetPlayerInterior(targetid, GetPlayerInterior(playerid));
	    SetPlayerVirtualWorld(targetid, GetPlayerVirtualWorld(playerid));
	    SetPlayerPos(targetid, pos[0], pos[1], pos[2]);
    }
	return 1;
}

YCMD:unban(playerid, params[], help)
{
	new query[300], name[24], id;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "s[24]", name)) return SendClientMessage(playerid, RED, "วิธีใช้: /unban [ชื่อผู้เล่น]");

    format(query, sizeof(query), "SELECT `MASTER_ID` FROM `CHARACTERS` WHERE `CHARACTER_NAME` = '%q' COLLATE NOCASE", name);
	database_result = db_query@(server_database, query);
	if(db_num_rows(database_result))
	{
		id = db_get_field_assoc_int(database_result, "MASTER_ID");
		db_free_result@(database_result);

		format(query, sizeof(query), "SELECT `MASTER_ID` FROM `BANS` WHERE `MASTER_ID` = '%i'", id);
		database_result = db_query@(server_database, query);
		if(db_num_rows(database_result))
		{
			db_free_result@(database_result);

			format(query, sizeof(query), "DELETE FROM `BANS` WHERE `MASTER_ID` = '%i'", id);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);

			return SendClientMessage(playerid, YELLOW, "เซิร์ฟเวอร์: คุณได้ปลดแบนผู้เล่นนี้แล้ว.");
		}
		else
		{
			SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนี้ไม่ได้ถูกแบน.");
		}
		db_free_result@(database_result);
	}
	else
	{
		SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นชื่อนี้ไม่ได้เชื่อมต่อกับบัญชีหลักใดๆ.");
	}
	db_free_result@(database_result);
	return 1;
}

YCMD:ban(playerid, params[], help)
{
	new query[600], string[144], targetid, days, reason[32], admin[24], player[24], timestamp = 0;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "uds[32]", targetid, days, reason)) return SendClientMessage(playerid, RED, "วิธีใช้: /ban [ชื่อผู้เล่น] [days] [reason]");
	if(strlen(reason) < 2 || strlen(reason) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: เหตุผลของคุณต้องมีความยาวระหว่าง 2-32 ตัวอักษร.");
	if(days < 0 || days > 365) return SendClientMessage(playerid, RED, "ล้มเหลว: จำนวนวันต้องอยู่ระหว่าง 0-365.");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถแบนตัวเองได้.");
	if(GetPlayerLevel(targetid) > PLAYER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถแบน staff กันเองได้.");

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	if(days > 0)
	{
		timestamp = gettime() + (86400 * days);
		format(string, sizeof(string), "แบน: [%s] %s (%i) ได้แบน %s (%i) เป็นเวลา %i วัน เนื่องจาก: %s", ReturnPlayerLevel(playerid), admin, playerid, player, targetid, days, reason);
	}
	else
	{
		format(string, sizeof(string), "แบน: [%s] %s (%i) ได้แบน %s (%i) เนื่องจาก: %s", ReturnPlayerLevel(playerid), admin, playerid, player, targetid, reason);
	}
	SendClientMessageToAll(RED, string);

	format(query, sizeof(query), "INSERT INTO `BANS` (`MASTER_ID`, `REASON`, `ADMIN`, `DAYS`, `TIMESTAMP`) VALUES ('%i', '%q', '%q', '%i', '%i')", GetPlayerMasterID(targetid), reason, admin, days, timestamp);
	database_result = db_query@(server_database, query);
	db_free_result@(database_result);
	return KickPlayer(targetid);
}

YCMD:kick(playerid, params[], help)
{
	new targetid, reason[32], string[144], admin[24], player[24];
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "us[32]", targetid, reason)) return SendClientMessage(playerid, RED, "วิธีใช้: /kick [ชื่อผู้เล่น] [เหตุผล]");
	if(strlen(reason) < 2 || strlen(reason) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: เหตุผลของคุณต้องมีความยาวระหว่าง 2-32 ตัวอักษร.");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถเตะตัวเองได้.");
	if(GetPlayerLevel(playerid) > PLAYER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถเตะ staff กันเองได้.");

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	format(string, sizeof(string), "KICK: [%s] %s (%i) has kicked %s (%i) for: %s", ReturnPlayerLevel(playerid), admin, playerid, player, targetid, reason);
	SendClientMessageToAll(RED, string);
	return KickPlayer(targetid);
}

YCMD:spec(playerid, params[], help)
{
	new targetid;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /spec [ชื่อผู้เล่น]");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");
	OnPlayerSpectate(playerid, targetid);
	return 1;
}

YCMD:unspec(playerid, params[], help)
{
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(StaffData[playerid][staff_spectating] == false) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ได้ดูผู้เล่นใดๆ.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");
    TogglePlayerSpectating(playerid, false);
    return SpawnPlayer(playerid);
}

YCMD:duty(playerid, params[], help)
{
	new string[128], name[24], Float:pos[3], Float:health, Float:armour;
    if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    if(IsPlayerSafe(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณอยู่ใน Safe Zone.");
    if(GetPlayerJailTime(playerid) > 0) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณถูกจับไว้ในเรือนจำ.");
    if(GetCameraStatus(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้เมื่อคุณกำลังใช้ Camera Mode.");
    if(StaffData[playerid][staff_spectating] == true) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้เมื่อคุณกำลังดูผู้เล่น.");
	if(IsPlayerInjured(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้ได้เมื่อคุณได้รับบาดเจ็บ.");

    GetPlayerName(playerid, name, sizeof(name));

	if(GetDutyStatus(playerid) == false)
    {
    	SetDutyStatus(playerid, true);

		GetPlayerHealth(playerid, health);
		SetStoredHealth(playerid, health);
		if(GetPlayerVIP(playerid)) { GetPlayerArmour(playerid, armour); SetStoredArmour(playerid, armour); }

        GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

        SetPlayerColor(playerid, ADMIN_COLOUR);

        SetPlayerHealth(playerid, MAX_HEALTH);
        SetPlayerArmour(playerid, 0.0);

        SetStoredWantedLevel(playerid, GetPlayerWantedLevel(playerid));
	
		StoreWeaponData(playerid);
		ResetPlayerWeapons(playerid);

        format(string, sizeof(string), "DUTY: [%s] %s (%i) ได้เปิดโหมดการทำงาน. [กำลังทำงาน ในโหมดผู้ดูแล]", ReturnPlayerLevel(playerid), name, playerid);
        return SendClientMessageToAll(GREY, string);
    }
    else if(GetDutyStatus(playerid) == true)
    {
        SetPlayerColourEx(playerid, GetPlayerJob(playerid));

        SetPlayerHealth(playerid, GetStoredHealth(playerid));
        SetPlayerArmour(playerid, GetStoredArmour(playerid));

        SetPlayerWantedLevel(playerid, GetStoredWantedLevel(playerid));

		RestoreStoredWeapons(playerid);

        SetDutyStatus(playerid, false);

       	format(string, sizeof(string), "DUTY: [%s] %s (%i) ได้ปิดโหมดการทำงาน. [กำลังเล่น ในโหมดปกติ]", ReturnPlayerLevel(playerid), name, playerid);
        SendClientMessageToAll(GREY, string);
    }
	return 1;
}

YCMD:h(playerid, params[], help)
{
	new question[100], name[24], string[144];
	if(GetPlayerMuted(playerid) == true) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถใช้คำสั่งนี้เมื่อคุณถูกปิดเสียง.");
	if(sscanf(params, "s[100]", question)) return SendClientMessage(playerid, RED, "วิธีใช้: /help [คำถาม]");
	if(strlen(question) < 1 || strlen(question) > 100) return SendClientMessage(playerid, RED, "ล้มเหลว: คำถามของคุณต้องมีความยาวระหว่าง 1-100 ตัวอักษร.");

	if((gettime() - 3) < GetTimerStatus(playerid, TIMER_TYPE_HELP)) return SendClientMessage(playerid, RED, "ล้มเหลว: โปรดรออีกสักครู่ก่อนที่จะส่งคำถามใหม่อีกครั้ง.");
	SetTimerStatus(playerid, TIMER_TYPE_HELP, gettime());

    GetPlayerName(playerid, name, sizeof(name));
	if(GetPlayerLevel(playerid) > PLAYER) { format(string, sizeof(string), "HELP(/h) [%s] %s: %s", ReturnPlayerLevel(playerid), name, question); }
	else { format(string, sizeof(string), "HELP(/h): [%s] %s: %s", ReturnPlayerStatus(playerid), name, question); }
	return SendClientMessageToAll(LIGHTBLUE, string);
}

YCMD:givevip(playerid, params[], help)
{
	new query[300], string[200], player[24], admin[24], targetid, id, vip;
	if(GetPlayerLevel(playerid) < OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับเจ้าของเซิร์ฟเวอร์เท่านั้น.");
	if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /givevip [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
    if(!IsPlayerLogged(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	GetPlayerName(targetid, player, sizeof(player));

	format(query, sizeof(query), "SELECT * FROM `CHARACTERS` WHERE `CHARACTER_NAME` = '%q' COLLATE NOCASE", player);
	database_result = db_query@(server_database, query);
	if(db_num_rows(database_result))
	{
		id = db_get_field_assoc_int(database_result, "MASTER_ID");
		vip = db_get_field_assoc_int(database_result, "VIP");
		db_free_result@(database_result);

		if(vip == 1) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นมีสถานะ V.I.P อยู่แล้ว.");

		format(query, sizeof(query), "SELECT `VIP` FROM `ACCOUNTS` WHERE `MASTER_ID` = '%i'", id);
		database_result = db_query@(server_database, query);
		if(db_num_rows(database_result))
		{
			db_free_result@(database_result);

			format(query, sizeof(query), "UPDATE `ACCOUNTS` SET VIP = '1' WHERE `MASTER_ID` = '%i'", id);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);

			GetPlayerName(playerid, admin, sizeof(admin));

			SetPlayerVIP(targetid, true);

			format(string, sizeof(string), "แอดมิน: %s %s (%i) ได้ให้สถานะ V.I.P ให้กับ %s.", ReturnPlayerLevel(playerid), admin, playerid, player);
			return SendClientMessageToAll(YELLOW, string);
		}
		else
		{
			SendClientMessage(playerid, RED, "ล้มเหลว: บัญชี้นี้ไม่มีอยู่ใน ฐานข้อมูล");
		}
		db_free_result@(database_result);
		return 1;
	}
	else
	{
		SendClientMessage(playerid, RED, "ล้มเหลว: บทบาทตัวละครนี้ไม่มีอยู่ใน ฐานข้อมูล");
	}
	db_free_result@(database_result);
	return 1;
}

YCMD:removevip(playerid, params[], help)
{
	new query[300], string[200], player[24], admin[24], targetid, id, vip;
	if(GetPlayerLevel(playerid) < OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับเจ้าของเซิร์ฟเวอร์เท่านั้น.");
	if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /removevip [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
    if(!IsPlayerLogged(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	GetPlayerName(targetid, player, sizeof(player));

	format(query, sizeof(query), "SELECT * FROM `CHARACTERS` WHERE `CHARACTER_NAME` = '%q' COLLATE NOCASE", player);
	database_result = db_query@(server_database, query);
	if(db_num_rows(database_result))
	{
		id = db_get_field_assoc_int(database_result, "MASTER_ID");
		vip = db_get_field_assoc_int(database_result, "VIP");
		db_free_result@(database_result);

		if(vip == 0) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่มีสถานะ V.I.P.");

		format(query, sizeof(query), "SELECT `VIP` FROM `ACCOUNTS` WHERE `MASTER_ID` = '%i'", id);
		database_result = db_query@(server_database, query);
		if(db_num_rows(database_result))
		{
			db_free_result@(database_result);

			format(query, sizeof(query), "UPDATE `ACCOUNTS` SET VIP = '0' WHERE `MASTER_ID` = '%i'", id);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);

			GetPlayerName(playerid, admin, sizeof(admin));

			SetPlayerVIP(targetid, false);

			format(string, sizeof(string), "แอดมิน: %s %s (%i) ได้ลบสถานะ V.I.P ของ %s.", ReturnPlayerLevel(playerid), admin, playerid, player);
			return SendClientMessageToAll(YELLOW, string);
		}
		else
		{
			SendClientMessage(playerid, RED, "ล้มเหลว: บัญชี้นี้ไม่มีอยู่ใน ฐานข้อมูล");
		}
		db_free_result@(database_result);
		return 1;
	}
	else
	{
		SendClientMessage(playerid, RED, "ล้มเหลว: บทบาทตัวละครนี้ไม่มีอยู่ใน ฐานข้อมูล");
	}
	db_free_result@(database_result);
	return 1;
}

YCMD:setlevel(playerid, params[], help)
{
	new query[300], string[128], name[24], admin[24], level[16], id, promotion;
	if(!IsPlayerAdmin(playerid))
	{
		if(GetPlayerLevel(playerid) != OWNER && GetPlayerLevel(playerid) != MANAGER) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับเจ้าของและผู้ดูแลเท่านั้น.");
	}
	if(sscanf(params, "s[24]s[16]", name, level)) return SendClientMessage(playerid, RED, "วิธีใช้: /setlevel [ชื่อผู้เล่น] [player/helper/admin/head/manager/owner]");

	if(!strcmp(level, "player", true)) { promotion = PLAYER; }
	else if(!strcmp(level, "helper", true)) { promotion = HELPER; }
	else if(!strcmp(level, "admin", true)) { promotion = ADMIN; }
	else if(!strcmp(level, "head", true)) { promotion = HEAD; }
	else if(!strcmp(level, "manager", true)) { if(!IsPlayerAdmin(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องเป็นผู้ดูแลระบบเพื่อเปลี่ยนเป็นระดับนี้."); promotion = MANAGER; }
	else if(!strcmp(level, "owner", true)) { if(!IsPlayerAdmin(playerid)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องเป็นผู้ดูแลระบบเพื่อเปลี่ยนเป็นระดับนี้."); promotion = OWNER; }
	else return SendClientMessage(playerid, RED, "ล้มเหลว: ระดับไม่ถูกต้อง. ต้องเป็น: player/helper/admin/head/manager/owner");

	format(query, sizeof(query), "SELECT `MASTER_ID` FROM `CHARACTERS` WHERE `CHARACTER_NAME` = '%q' COLLATE NOCASE", name);
	database_result = db_query@(server_database, query);
	if(db_num_rows(database_result))
	{
		id = db_get_field_assoc_int(database_result, "MASTER_ID");
		db_free_result@(database_result);

		format(query, sizeof(query), "SELECT `LEVEL` FROM `ACCOUNTS` WHERE `MASTER_ID` = '%i'", id);
		database_result = db_query@(server_database, query);
		if(db_num_rows(database_result))
		{
			db_free_result@(database_result);

			format(query, sizeof(query), "UPDATE `ACCOUNTS` SET LEVEL = '%i' WHERE `MASTER_ID` = '%i'", promotion, id);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);

			GetPlayerName(playerid, admin, sizeof(admin));

			new targetid = GetID(name);
			if(targetid != INVALID_PLAYER_ID)
			{
				SetPlayerLevel(targetid, promotion);
			}
			
			format(string, sizeof(string), "แอดมิน: %s %s (%i) ได้เปลี่ยนระดับของ %s เป็น: %s", ReturnPlayerLevel(playerid), admin, playerid, name, ReturnLevelName(promotion));
			return SendClientMessageToAll(YELLOW, string);
		}
		else
		{
			SendClientMessage(playerid, RED, "ล้มเหลว: บัญชีนี้ไม่มีอยู่ใน ฐานข้อมูล");
		}
		db_free_result@(database_result);
		return 1;
	}
	else
	{
		SendClientMessage(playerid, RED, "ล้มเหลว: บทบาทตัวละครนี้ไม่มีอยู่ใน ฐานข้อมูล");
	}
	db_free_result@(database_result);
	return 1;
}

YCMD:move(playerid, params[], help)
{
	new direction[16], Float:distance, Float:pos[3];
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(sscanf(params, "s[16]f", direction, distance)) return SendClientMessage(playerid, RED, "วิธีใช้: /move [north/south/east/west/up/down] [distance]");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

    if(!strcmp(direction, "north", true)) { SetPlayerPos(playerid, pos[0], pos[1] + distance, pos[2]); }
    else if(!strcmp(direction, "south", true)) { SetPlayerPos(playerid, pos[0], pos[1] - distance, pos[2]); }
    else if(!strcmp(direction, "east", true)) { SetPlayerPos(playerid, pos[0] + distance, pos[1], pos[2]); }
    else if(!strcmp(direction, "west", true)) { SetPlayerPos(playerid, pos[0] - distance, pos[1], pos[2]); }
    else if(!strcmp(direction, "up", true)) { SetPlayerPos(playerid, pos[0], pos[1], pos[2] + distance); }
    else if(!strcmp(direction, "down", true)) { SetPlayerPos(playerid, pos[0], pos[1], pos[2] - distance); }
	else
	{
		SendClientMessage(playerid, RED, "ล้มเหลว: ทิศทางไม่ถูกต้อง. ต้องเป็น: north/south/east/west/up/down");
	}
	return 1;
}

YCMD:admins(playerid, params[], help)
{
	new query[800], total = 0, online = 0, string[200], name[24], level = PLAYER, id;
    format(query, sizeof(query), "SELECT * FROM `ACCOUNTS` WHERE `LEVEL` > '%i'", PLAYER);
    database_result = db_query@(server_database, query);
    if((total = db_num_rows(database_result)))
    {
        query[0] = EOS;
        do
        {
	        db_get_field_assoc@(database_result, "JOIN_NAME", name, sizeof(name));
	        
	      	id = db_get_field_assoc_int(database_result, "MASTER_ID");
	      	level = db_get_field_assoc_int(database_result, "LEVEL");

            if(CheckIfAdminOnline(id) != INVALID_PLAYER_ID)
            {
                format(string, sizeof(string), "{FFFFFF}[%s] %s\n", ReturnLevelName(level), name);
                online++;
            }
            else
			{
				format(string, sizeof(string), "{808080}[%s] %s\n", ReturnLevelName(level), name);
			}
			strcat(query, string);
        }
        while(db_next_row(database_result));
    }
    db_free_result@(database_result);

    if(total == 0) return SendClientMessage(playerid, RED, "ล้มเหลว: ไม่มีผู้ดูแลใดๆ ในฐานข้อมูล.");

	format(string, sizeof(string), "{FFFFFF}%s แอดมิน (%i/%i ออนไลน์)", SERVER_TAG, online, total);
	return ShowPlayerDialog(playerid, LIST_DIALOG, DIALOG_STYLE_LIST, string, query, "ปิด", "");
}

YCMD:time(playerid, params[], help)
{
	new string[128], name[24], time;
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(sscanf(params, "i", time)) return SendClientMessage(playerid, RED, "วิธีใช้: /time [0-23]");
	if(time < 0 || time > 23) return SendClientMessage(playerid, RED, "ล้มเหลว: เวลาไม่ถูกต้อง. ต้องเป็น: 0-23");

	GetPlayerName(playerid, name, sizeof(name));

	format(string, sizeof(string), "เวลา: %s %s (%i) ได้ถูกเปลี่ยนเป็นเวลา: %i น.", ReturnPlayerLevel(playerid), name, playerid, time);
	SendClientMessageToAll(WHITE, string);

	ElementsData[server_time] = time;
	return SetWorldTime(ElementsData[server_time]);
}

YCMD:weather(playerid, params[], help)
{
	new string[128], name[24], weather;
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(sscanf(params, "i", weather)) return SendClientMessage(playerid, RED, "วิธีใช้: /weather [0-20]");
	if(weather < 0 || weather > 20) return SendClientMessage(playerid, RED, "ล้มเหลว: สภาพอากาศไม่ถูกต้อง. ต้องเป็น: 0-20");

	GetPlayerName(playerid, name, sizeof(name));

	format(string, sizeof(string), "ภาพอากาศ: %s %s (%i) ได้เปลี่ยนสภาพอากาศเป็น: %i", ReturnPlayerLevel(playerid), name, playerid, weather);
	SendClientMessageToAll(WHITE, string);

	return SetWeather(weather);
}

YCMD:giveitem(playerid, params[], help)
{
	new targetid;
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(sscanf(params, "u", targetid)) return SendClientMessage(playerid, RED, "วิธีใช้: /giveitem [ชื่อผู้เล่น]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	StaffData[playerid][staff_giveid] = targetid;

	return ShowPlayerDialog(playerid, GIVE_ITEM_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}Give Item", "{FFFFFF}Sprunk\nAlcohol\nBurger\nTaco\nPizza\nMedikit\nToolbox\nGun Materials\nJerrycan\nCrowbar\nLotto Ticket\nSeeds\nWeed\nCell Phone", "Give", "Cancel");
}

YCMD:giveweapon(playerid, params[], help)
{
	new string[200], admin[24], player[24], name[32], targetid, weapon, ammo;
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
	if(sscanf(params, "uii", targetid, weapon, ammo)) return SendClientMessage(playerid, RED, "วิธีใช้: /giveweapon [ชื่อผู้เล่น] [อาวุธ] [จำนวนกระสุน]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นนี้ไม่ได้ออนไลน์.");
    if(!IsPlayerSpawned(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นยังไม่ได้เกิด.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นคนนั้นเป็น NPC.");

	if(ammo < 1 || ammo > 5000) return SendClientMessage(playerid, RED, "ล้มเหลว: Invalid ammo amount. Must be from: 1-5000");

	for(new w; w < sizeof(BannableWeapons); w++)
  	{
    	if(weapon == BannableWeapons[w])
   		{
        	 return SendClientMessage(playerid, RED, "ล้มเหลว: อาวุธ ID นี้ถูกห้ามใช้งาน.");
   		}
	}

	GetPlayerName(playerid, admin, sizeof(admin));
	GetPlayerName(targetid, player, sizeof(player));

	GetWeaponName(weapon, name, sizeof(name));

	if(targetid != playerid)
	{
		format(string, sizeof(string), "แอดมิน: [%s] %s (%i) ได้ให้คุณ %s อาวุธ %s พร้อมกระสุน %i นัด.", ReturnPlayerLevel(playerid), admin, playerid, name, weapon, ammo);
		SendClientMessage(targetid, YELLOW, string);
	}

	format(string, sizeof(string), "แอดมิน: คุณได้ให้ %s (%i) อาวุธ %s พร้อมกระสุน %i นัด.", player, targetid, name, ammo);
	SendClientMessage(playerid, YELLOW, string);
	return GivePlayerWeapon(targetid, weapon, ammo);
}

YCMD:gotols(playerid, params[], help)
{
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    SetPlayerPos(playerid,1529.6, -1691.2, 13.3);
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
    return SendClientMessage(playerid, GREEN, "ระบบ: คุณได้ย้ายไปยัง Los Santos แล้ว");
}

YCMD:gotosf(playerid, params[], help)
{
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    SetPlayerPos(playerid, -1930.3608, 198.0463, 25.6393);
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
    return SendClientMessage(playerid, GREEN, "ระบบ: คุณได้ย้ายไปยัง San Fierro แล้ว");
}

YCMD:gotolv(playerid, params[], help)
{
	if(GetPlayerLevel(playerid) < ADMIN) return SendClientMessage(playerid, RED, "ล้มเหลว: คำสั่งนี้ถูกจำกัดไว้สำหรับผู้ดูแลและขึ้นไป.");
    SetPlayerPos(playerid, 1699.2, 1435.1, 10.7);
	SetPlayerInterior(playerid, 0);
	SetPlayerVirtualWorld(playerid, 0);
    return SendClientMessage(playerid, GREEN, "ระบบ: คุณได้ย้ายไปยัง Las Venturas แล้ว");
}