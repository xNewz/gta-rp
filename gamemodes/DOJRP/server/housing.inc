#include <YSI_Coding\y_hooks>

new Iterator:Houses<MAX_HOUSES>;

forward EncryptHousePassword(playerid, houseid);
forward VerifyHousePassword(playerid, bool:success);

enum house_data
{
	house_charid,
	house_city,
	house_value,
	house_safe,
	Float:house_extx,
	Float:house_exty,
	Float:house_extz,
	Float:house_intx,
	Float:house_inty,
	Float:house_intz,
	Float:house_enterx,
	Float:house_entery,
	Float:house_enterz,
	Float:house_entera,
	Float:house_exitx,
	Float:house_exity,
	Float:house_exitz,
	Float:house_exita,
	house_extinterior,
	house_extworld,
	house_intinterior,
	house_intworld,
	house_mapicon,
	house_entercp,
	house_exitcp,
	bool:house_active,
	bool:house_onfire
};
new HouseData[MAX_HOUSES][house_data];

enum tenant_data
{
	tenant_houseid,
	tenant_salehouse,
	tenant_saleprice,
	tenant_saleowner,
	tenant_saleto,
	bool:tenant_saleactive
};
new TenantData[MAX_PLAYERS][tenant_data];

stock GetHouseFireStatus(houseid) return HouseData[houseid][house_onfire];
stock SetHouseFireStatus(houseid, bool:status) return HouseData[houseid][house_onfire] = status;

stock Float:GetHouseExtX(houseid) return HouseData[houseid][house_extx];
stock Float:GetHouseExtY(houseid) return HouseData[houseid][house_exty];
stock Float:GetHouseExtZ(houseid) return HouseData[houseid][house_extz];

stock RemoveHouseOwnerFromDatabase(characterid)
{
	new query[300], password[64];
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true && characterid == HouseData[i][house_charid])
	 	{
			format(password, sizeof(password), "$2y$12$1h2ra6euo5IoIGlVWgvnN.kIOiImlQRnML7Zw/GDZ6Ogb89kA9Lpe");//Randomized Bcrypt Password

		 	HouseData[i][house_charid] = 0;
			HouseData[i][house_safe] = 0;

			DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
			HouseData[i][house_mapicon] = CreateDynamicMapIcon(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);

			format(query, sizeof(query), "UPDATE `HOUSES` SET `CHARACTER_ID` = '%i', `PASS` = '%s', `SAFE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_charid], password, HouseData[i][house_safe], i);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);
		}
	}
	return 1;
}

stock PointInRangeOfPoint(Float:range, Float:x, Float:y, Float:z, Float:x2, Float:y2, Float:z2)
{
    x2 -= x;
    y2 -= y;
    z2 -= z;
    return ((x2 * x2) + (y2 * y2) + (z2 * z2)) < (range * range);
}

stock ReturnHousePercent(amount, percent)
{
	return (amount / 100 * percent);
}

stock GetFreeHouseSlot()
{
	for(new i = 0; i < MAX_HOUSES; i++)
	{
	    if(HouseData[i][house_active] == false) return i;
	}
	return -1;
}

stock GetOwnedHouseID(playerid)
{
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true && GetPlayerCharacterID(playerid) == HouseData[i][house_charid])
	 	{
			if(IsPlayerInRangeOfPoint(playerid, 100.0, HouseData[i][house_intx], HouseData[i][house_inty], HouseData[i][house_intz]) && GetPlayerInterior(playerid) == HouseData[i][house_intinterior] && GetPlayerVirtualWorld(playerid) == HouseData[i][house_intworld]) return i;
		}
	}
	return -1;
}

stock ReturnHouseID(playerid)
{
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	 	{
			if(IsPlayerInRangeOfPoint(playerid, 100.0, HouseData[i][house_intx], HouseData[i][house_inty], HouseData[i][house_intz]) && GetPlayerInterior(playerid) == HouseData[i][house_intinterior] && GetPlayerVirtualWorld(playerid) == HouseData[i][house_intworld]) return i;
		}
	}
	return -1;
}

stock GetHouseDoor(houseid, &Float:x, &Float:y, &Float:z)
{
	x = HouseData[houseid][house_extx];
	y = HouseData[houseid][house_exty];
	z = HouseData[houseid][house_extz];
	return 1;
}

stock UpdateNearbyLandValue(houseid)
{
	new query[200];
    foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true && i != houseid)
		{
			if(PointInRangeOfPoint(100.0, HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz]))
			{
				HouseData[i][house_value] += ReturnHousePercent(HouseData[i][house_value], LAND_VALUE_PERCENT);

			 	format(query, sizeof(query), "UPDATE `HOUSES` SET `VALUE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_value], i);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);
		    }
	    }
	}
	return 1;
}

stock ShowHouseDialog(playerid)
{
	return ShowPlayerDialog(playerid, MENU_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}House Menu", "{FFFFFF}Access Safe\nChange Password\nSell House", "Select", "Cancel");
}

stock ShowSafeDialog(playerid)
{
	return ShowPlayerDialog(playerid, SAFE_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}House Safe > Menu", "{FFFFFF}Balance\nDeposit\nWithdraw", "Select", "Back");
}

stock GetRandomHouseID()
{
    new house_pool[MAX_HOUSES], idx, houseid = -1;
    foreach(new i : Houses)
    {
        if(HouseData[i][house_active] == true)
        {
        	if(HouseData[i][house_onfire] == true) continue;
        	house_pool[idx] = i;
        	idx++;
    	}
    }
	
	if(idx > 0)
	{
   		houseid = house_pool[random(idx)];
  	}
    return houseid;
}

stock GetOwnedHouseCount(playerid)
{
	new count = 0;
	foreach(new i : Houses)
    {
    	if(HouseData[i][house_active] == true)
    	{
	    	if(GetPlayerCharacterID(playerid) == HouseData[i][house_charid])
	        {
	        	count++;
	        }
    	}
    }
	return count;
}

stock GetHouseCity(houseid)
{
	return HouseData[houseid][house_city];
}

stock LoadHouses()
{
	new query[200];
	for(new i = 0; i < MAX_HOUSES; i++)
	{
		format(query, sizeof(query), "SELECT * FROM `HOUSES` WHERE `HOUSE_ID` = '%i'", i);
		database_result = db_query@(server_database, query);
		if(db_num_rows(database_result))
		{
			HouseData[i][house_charid] = db_get_field_assoc_int(database_result, "CHARACTER_ID");
			HouseData[i][house_city] = db_get_field_assoc_int(database_result, "CITY");
			HouseData[i][house_value] = db_get_field_assoc_int(database_result, "VALUE");
			HouseData[i][house_safe] = db_get_field_assoc_int(database_result, "SAFE");

			HouseData[i][house_extx] = db_get_field_assoc_float(database_result, "EXTX");
			HouseData[i][house_exty] = db_get_field_assoc_float(database_result, "EXTY");
			HouseData[i][house_extz] = db_get_field_assoc_float(database_result, "EXTZ");

			HouseData[i][house_intx] = db_get_field_assoc_float(database_result, "INTX");
			HouseData[i][house_inty] = db_get_field_assoc_float(database_result, "INTY");
			HouseData[i][house_intz] = db_get_field_assoc_float(database_result, "INTZ");

			HouseData[i][house_enterx] = db_get_field_assoc_float(database_result, "ENTERX");
			HouseData[i][house_entery] = db_get_field_assoc_float(database_result, "ENTERY");
			HouseData[i][house_enterz] = db_get_field_assoc_float(database_result, "ENTERZ");
			HouseData[i][house_entera] = db_get_field_assoc_float(database_result, "ENTERA");

			HouseData[i][house_exitx] = db_get_field_assoc_float(database_result, "EXITX");
			HouseData[i][house_exity] = db_get_field_assoc_float(database_result, "EXITY");
			HouseData[i][house_exitz] = db_get_field_assoc_float(database_result, "EXITZ");
			HouseData[i][house_exita] = db_get_field_assoc_float(database_result, "EXITA");

			HouseData[i][house_extinterior] = db_get_field_assoc_int(database_result, "EXTINTERIOR");
			HouseData[i][house_extworld] = db_get_field_assoc_int(database_result, "EXTWORLD");

			HouseData[i][house_intinterior] = db_get_field_assoc_int(database_result, "INTINTERIOR");
			HouseData[i][house_intworld] = db_get_field_assoc_int(database_result, "INTWORLD");

			if(HouseData[i][house_charid] == 0)
			{
				HouseData[i][house_mapicon] = CreateDynamicMapIcon(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);
			}
			else
			{
				HouseData[i][house_mapicon] = CreateDynamicMapIcon(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 32, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);
			}

			HouseData[i][house_entercp] = CreateDynamicCP(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 1.0, HouseData[i][house_extworld], HouseData[i][house_extinterior], -1, 4.0);
			HouseData[i][house_exitcp] = CreateDynamicCP(HouseData[i][house_intx], HouseData[i][house_inty], HouseData[i][house_intz], 1.0, HouseData[i][house_intworld], HouseData[i][house_intinterior], -1, 4.0);

			HouseData[i][house_active] = true;

			Iter_Add(Houses, i);
		}
		db_free_result@(database_result);
	}
	return 1;
}

stock UnloadHouses()
{
	foreach(new i : Houses)
	{
		DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
		DestroyDynamicCP(HouseData[i][house_entercp]);
		DestroyDynamicCP(HouseData[i][house_exitcp]);

		Iter_Remove(Houses, i);
	}
	return 1;
}

hook OnPlayerConnect(playerid)
{
	if(!IsPlayerNPC(playerid))
	{
		TenantData[playerid][tenant_houseid] = -1;
		TenantData[playerid][tenant_salehouse] = -1;
		TenantData[playerid][tenant_saleprice] = 0;
		TenantData[playerid][tenant_saleowner] = INVALID_PLAYER_ID;
		TenantData[playerid][tenant_saleto] = INVALID_PLAYER_ID;
		TenantData[playerid][tenant_saleactive] = false;
	}
	return 1;
}

hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid)
	{
		case BUY_DIALOG:
	    {
	        if(response)
	        {
	            new string[128], houseid = TenantData[playerid][tenant_houseid];
		   		format(string, sizeof(string), "{FFFFFF}คุณแน่ใจหรือไม่ว่าต้องการซื้อบ้านนี้ในราคา $%i?", HouseData[houseid][house_value]);
	            return ShowPlayerDialog(playerid, VERIFY_DIALOG, DIALOG_STYLE_MSGBOX, "{FFFFFF}ยืนยันการซื้อบ้าน", string, "ยืนยัน", "ยกเลิก");
	        }
	        else
	        {
	        	new houseid = TenantData[playerid][tenant_houseid];
				SetPlayerInterior(playerid, HouseData[houseid][house_intinterior]);
				SetPlayerVirtualWorld(playerid, HouseData[houseid][house_intworld]);
   	    	  	SetPlayerPos(playerid, HouseData[houseid][house_enterx], HouseData[houseid][house_entery], HouseData[houseid][house_enterz]);
   	    	  	SetPlayerFacingAngle(playerid, HouseData[houseid][house_entera]);
   	    	  	return SetCameraBehindPlayer(playerid);
	        }
	    }
	    case VERIFY_DIALOG:
	    {
	        if(response)
	        {
	            new password[64], string[200], query[300], houseid = TenantData[playerid][tenant_houseid], count = GetOwnedHouseCount(playerid);
	            if(GetPlayerMoneyEx(playerid) < HouseData[houseid][house_value]) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่มีเงินเพียงพอสำหรับการซื้อบ้านนี้.");
	            
	           	if(GetPlayerVIP(playerid))
				{
				  	if(count >= MAX_VIP_HOUSES)
				  	{
				  		format(string, sizeof(string), "ล้มเหลว: คุณสามารถเป็นเจ้าของบ้านได้สูงสุด x%i หลัง กรุณาขายบ้านก่อน.", MAX_VIP_HOUSES);
				  		return SendClientMessage(playerid, RED, string);
				  	}
				}
				else 
				{
					if(count >= MAX_PLAYER_HOUSES)
					{
				  		format(string, sizeof(string), "ล้มเหลว: คุณสามารถเป็นเจ้าของบ้านได้สูงสุด x%i หลัง กรุณาขายบ้านก่อน.", MAX_PLAYER_HOUSES);
				  		return SendClientMessage(playerid, RED, string);
					}
				}
	            
	            GivePlayerMoney(playerid, -HouseData[houseid][house_value]);
	            
	            UpdateNearbyLandValue(houseid);
				
				DestroyDynamicMapIcon(HouseData[houseid][house_mapicon]);
				HouseData[houseid][house_mapicon] = CreateDynamicMapIcon(HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], 32, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);
				
				SetPlayerPos(playerid, HouseData[houseid][house_exitx], HouseData[houseid][house_exity], HouseData[houseid][house_exitz]);
				SetPlayerFacingAngle(playerid, HouseData[houseid][house_exita] + 180);
				SetCameraBehindPlayer(playerid);
				
				format(password, sizeof(password), "$2y$12$1h2ra6euo5IoIGlVWgvnN.kIOiImlQRnML7Zw/GDZ6Ogb89kA9Lpe");//Randomized Bcrypt Password

				new id = GetPlayerCharacterID(playerid);
				HouseData[houseid][house_charid] = id;

				format(query, sizeof(query), "UPDATE `HOUSES` SET `CHARACTER_ID` = '%i', `PASS` = '%s' WHERE `HOUSE_ID` = '%i'", HouseData[houseid][house_charid], password, houseid);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);
	        }
	        return 1;
		}
	    case ACCESS_DIALOG:
	    {
	        if(response)
	        {
	        	new query[200], password[64], houseid = TenantData[playerid][tenant_houseid];
	            if(strlen(inputtext) < 3 || strlen(inputtext) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: รหัสผ่านต้องมีความยาวระหว่าง 3-32 ตัวอักษร.");

	            format(query, sizeof(query), "SELECT `PASS` FROM `HOUSES` WHERE `HOUSE_ID` = '%i'", houseid);
				database_result = db_query@(server_database, query);
				if(db_num_rows(database_result))
				{
					db_get_field_assoc@(database_result, "PASS", password, sizeof(password));
					bcrypt_verify(playerid, "VerifyHousePassword", inputtext, password);
				}
				else
				{
					SendClientMessage(playerid, RED, "ล้มเหลว: เกิดข้อผิดพลาดขณะตรวจสอบรหัสผ่าน.");
				}
				db_free_result@(database_result);
	        }
	        return 1;
	    }
	    case MENU_DIALOG:
	    {
	        if(response)
	        {
	            switch(listitem)
	            {
	                case 0:
	                {
	                    return ShowPlayerDialog(playerid, SAFE_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}House Menu > Safe", "{FFFFFF}Balance\nDeposit\nWithdraw", "Select", "Back");
	                }
	                case 1:
	                {
	                    return ShowPlayerDialog(playerid, PASS_DIALOG, DIALOG_STYLE_PASSWORD, "{FFFFFF}House Menu > Change Password", "{FFFFFF}โปรดป้อนรหัสผ่านใหม่:", "Enter", "Cancel");
	                }
	                case 2:
	                {
	                    new string[128], houseid = TenantData[playerid][tenant_houseid];
	                    format(string, sizeof(string), "{FFFFFF}คุณต้องการขายบ้านของคุณในราคา $%i?", HouseData[houseid][house_value]);
	                    return ShowPlayerDialog(playerid, SELL_DIALOG, DIALOG_STYLE_MSGBOX, "{FFFFFF}House Menu > Sell House", string, "Yes", "No");
	                }
				}
	        }
	        return 1;
		}
	    case VALUE_DIALOG:
	    {
	        if(!response)
	        {
	            ShowHouseDialog(playerid);
	        }
	        return 1;
		}
	    case PASS_DIALOG:
	    {
	        if(response)
	        {
	            if(strlen(inputtext) < 3 || strlen(inputtext) > 32) return SendClientMessage(playerid, RED, "ล้มเหลว: รหัสผ่านต้องมีความยาวระหว่าง 3-32 ตัวอักษร.");
				bcrypt_hash(playerid, "EncryptHousePassword", inputtext, 12, "i", TenantData[playerid][tenant_houseid]);
	        }
	        else
	        {
	        	ShowHouseDialog(playerid);
	        }
	        return 1;
		}
	    case SAFE_DIALOG:
	    {
	        if(response)
	        {
	            new string[64], houseid = TenantData[playerid][tenant_houseid];
	            switch(listitem)
	            {
	                case 0:
	                {
	                    format(string, sizeof(string), "{FFFFFF}Funds: $%i", HouseData[houseid][house_safe]);
	                    return ShowPlayerDialog(playerid, SAFE_BALANCE_DIALOG, DIALOG_STYLE_LIST, "{FFFFFF}House Menu > Safe", string, "Back", "");
	                }
	                case 1:
	                {
	                    format(string, sizeof(string), "{FFFFFF}House Menu > Safe (Pocket: $%i)", GetPlayerMoneyEx(playerid));
	                    return ShowPlayerDialog(playerid, SAFE_DEPOSIT_DIALOG, DIALOG_STYLE_INPUT, string, "{FFFFFF}คุณต้องการฝากเงินเท่าไหร่?", "ฝาก", "ยกเลิก");
	                }
	                case 2:
	                {
	                    format(string, sizeof(string), "{FFFFFF}House Menu > Safe (Funds: $%i)", HouseData[houseid][house_safe]);
	                    return ShowPlayerDialog(playerid, SAFE_WITHDRAW_DIALOG, DIALOG_STYLE_INPUT, string, "{FFFFFF}ต้องการถอนเงินเท่าไหร่?", "ถอน", "ยกเลิก");
	                }
				}
	        }
	        else
	        {
	        	ShowHouseDialog(playerid);
	        }
	        return 1;
		}
	    case SAFE_BALANCE_DIALOG:
	    {
	        if(response)
	        {
	            ShowSafeDialog(playerid);
	        }
	        return 1;
		}
	    case SAFE_DEPOSIT_DIALOG:
	    {
	        if(response)
	        {
	            if(!IsNumeric(inputtext) || strval(inputtext) < 1) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องใส่ตัวเลขมากกว่า 0.");
	            if(GetPlayerMoneyEx(playerid) < strval(inputtext)) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่มีเงินเพียงพอในกระเป๋าเงินของคุณ.");
	            
	            GivePlayerMoney(playerid, -strval(inputtext));
	            
	            new string[128], query[128], houseid = TenantData[playerid][tenant_houseid];
	            HouseData[houseid][house_safe] += strval(inputtext);

				format(query, sizeof(query), "UPDATE `HOUSES` SET `SAFE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[houseid][house_safe], houseid);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);
	            
	            format(string, sizeof(string), "ระบบ: คุณได้ฝากเงินเข้าเซฟของคุณเรียบร้อยแล้ว $%i.", strval(inputtext));
	            return SendClientMessage(playerid, YELLOW, string);
	        }
	        else
	        {
	            ShowSafeDialog(playerid);
	        }
	        return 1;
		}
	    case SAFE_WITHDRAW_DIALOG:
	    {
	        if(response)
	        {
	            if(!IsNumeric(inputtext) || strval(inputtext) < 1) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องใส่ตัวเลขมากกว่า 0.");
	            
	            new string[128], query[128], houseid = TenantData[playerid][tenant_houseid];
	            if(strval(inputtext) > HouseData[houseid][house_safe]) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่มีเงินในเซฟเพียงพอ.");

	            GivePlayerMoney(playerid, strval(inputtext));

	            HouseData[houseid][house_safe] -= strval(inputtext);

				format(query, sizeof(query), "UPDATE `HOUSES` SET `SAFE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[houseid][house_safe], houseid);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);
	            
	            format(string, sizeof(string), "ระบบ: คุณได้ถอนเงินจากเซฟของคุณเรียบร้อยแล้ว $%i.", strval(inputtext));
	            return SendClientMessage(playerid, YELLOW, string);
	        }
	        else
	        {
	            ShowSafeDialog(playerid);
	        }
	        return 1;
		}
		case SELL_DIALOG:
		{
	        if(response)
	        {
	            new string[128], query[200], houseid = TenantData[playerid][tenant_houseid];
	            GivePlayerMoney(playerid, (HouseData[houseid][house_value]));
				
				HouseData[houseid][house_charid] = 0;

				format(query, sizeof(query), "UPDATE `HOUSES` SET  `CHARACTER_ID` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[houseid][house_charid], houseid);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);
				
				DestroyDynamicMapIcon(HouseData[houseid][house_mapicon]);
				HouseData[houseid][house_mapicon] = CreateDynamicMapIcon(HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);
	        	
	            format(string, sizeof(string), "ระบบ: คุณได้ขายบ้านของคุณเรียบร้อยแล้ว ราคา $%i.", HouseData[houseid][house_value]);
	            SendClientMessage(playerid, YELLOW, string);
	        }
	        return 1;
		}
	}
	return 1;
}

hook OnPlayerEnterDynamicCP(playerid, checkpointid)
{
	new string[128];
	if(GetPlayerState(playerid) == PLAYER_STATE_ONFOOT)
	{
		foreach(new i : Houses)
		{
			if(HouseData[i][house_active] == true)
			{
				if(checkpointid == HouseData[i][house_entercp])
				{
			   	 	if(HouseData[i][house_charid] == 0)
					{
					 	if(HouseData[i][house_onfire] == true) return SendClientMessage(playerid, ORANGE, "เซิร์ฟเวอร์: บ้านหลังนี้กำลังไฟไหม้ กลับมาใหม่เมื่อปลอดภัยแล้ว.");

						TenantData[playerid][tenant_houseid] = i;

					 	format(string, sizeof(string), "{FFFFFF}ราคา: $%i", HouseData[i][house_value]);
						return ShowPlayerDialog(playerid, BUY_DIALOG, DIALOG_STYLE_MSGBOX, string, "{FFFFFF}คุณต้องการซื้อหรือดูตัวอย่างบ้านนี้หรือไม่?", "ซื้อ", "ดูตัวอย่าง");
					}
					else if(GetPlayerCharacterID(playerid) == HouseData[i][house_charid])
					{
						if(HouseData[i][house_onfire] == true) return SendClientMessage(playerid, ORANGE, "เซิร์ฟเวอร์: บ้านหลังนี้กำลังไฟไหม้ กลับมาใหม่เมื่อปลอดภัยแล้ว.");

					 	TenantData[playerid][tenant_houseid] = i;

						SetPlayerInterior(playerid, HouseData[i][house_intinterior]);
						SetPlayerVirtualWorld(playerid, HouseData[i][house_intworld]);
			   	    	SetPlayerPos(playerid, HouseData[i][house_enterx], HouseData[i][house_entery], HouseData[i][house_enterz]);
			   	    	SetPlayerFacingAngle(playerid, HouseData[i][house_entera]);
			   	    	SetCameraBehindPlayer(playerid);

			   	    	return SendClientMessage(playerid, WHITE, "เซิร์ฟเวอร์: พิมพ์ /hmenu เพื่อเข้าสู่เมนูบ้านของคุณ.");
					}
					else
					{
					 	if(HouseData[i][house_onfire] == true) return SendClientMessage(playerid, ORANGE, "เซิร์ฟเวอร์: บ้านหลังนี้กำลังไฟไหม้ กลับมาใหม่เมื่อปลอดภัยแล้ว.");

					  	TenantData[playerid][tenant_houseid] = i;

						format(string, sizeof(string), "{FFFFFF}Owner: %s (Worth: $%i)", ReturnCharacterName(HouseData[i][house_charid]), HouseData[i][house_value]);
						ShowPlayerDialog(playerid, ACCESS_DIALOG, DIALOG_STYLE_PASSWORD, string, "{FFFFFF}โปรดป้อนรหัสผ่าน:", "Enter", "Cancel");
					}
					return 1;
			 	}
			  	else if(checkpointid == HouseData[i][house_exitcp])
			  	{
					SetPlayerInterior(playerid, HouseData[i][house_extinterior]);
					SetPlayerVirtualWorld(playerid, HouseData[i][house_extworld]);

			   		SetPlayerPos(playerid, HouseData[i][house_exitx], HouseData[i][house_exity], HouseData[i][house_exitz]);
			   	 	SetPlayerFacingAngle(playerid, HouseData[i][house_exita]);
			  	 	return SetCameraBehindPlayer(playerid);
			  	}
		 	}
	 	}
 	}
	return 1;
}

hook EncryptHousePassword(playerid, houseid)
{
	new query[200], password[64];
	bcrypt_get_hash(password);

	format(query, sizeof(query), "UPDATE `HOUSES` SET `PASS` = '%s' WHERE `HOUSE_ID` = '%i'", password, houseid);
	database_result = db_query@(server_database, query);
	db_free_result@(database_result);
	
	return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้เปลี่ยนรหัสผ่านของบ้านของคุณเรียบร้อยแล้ว.");
}

hook VerifyHousePassword(playerid, bool:success)
{
 	if(success)
	{
	    new houseid = TenantData[playerid][tenant_houseid];
		SetPlayerInterior(playerid, HouseData[houseid][house_intinterior]);
		SetPlayerVirtualWorld(playerid, HouseData[houseid][house_intworld]);
   	  	SetPlayerPos(playerid, HouseData[houseid][house_enterx], HouseData[houseid][house_entery], HouseData[houseid][house_enterz]);
   	   	SetPlayerFacingAngle(playerid, HouseData[houseid][house_entera]);
   	 	return SetCameraBehindPlayer(playerid);
 	}
	else
 	{
 		SendClientMessage(playerid, ORANGE, "ระบบ: รหัสผ่านไม่ถูกต้อง ติดต่อเจ้าของบ้านเพื่อขอรหัสผ่าน.");
 	}
	return 1;
}

//Player Commands
YCMD:hmenu(playerid, params[], help)
{
	new houseid = GetOwnedHouseID(playerid);
	if(houseid == -1) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในบ้านที่คุณเป็นเจ้าของเพื่อใช้คำสั่งนี้.");
	TenantData[playerid][tenant_houseid] = houseid;
	return ShowHouseDialog(playerid);
}

YCMD:sellhouse(playerid, params[], help)
{
	new Float:pos[3], string[200], targetid, price, houseid = GetOwnedHouseID(playerid), name[24];
	if(houseid == -1) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในบ้านที่คุณเป็นเจ้าของเพื่อใช้คำสั่งนี้.");
	if(sscanf(params, "ui", targetid, price)) return SendClientMessage(playerid, RED, "วิธีใช้: /sellhouse [ชื่อผู้เล่น] [ราคา]");
	if(!IsPlayerConnected(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นนั้นไม่ได้ออนไลน์.");
	if(IsPlayerNPC(targetid)) return SendClientMessage(playerid, RED, "ล้มเหลว: ผู้เล่นนั้นเป็น NPC และไม่สามารถเป็นเจ้าของบ้านได้.");
	if(targetid == playerid) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่สามารถขายบ้านให้ตัวเองได้.");
	if(price < 1) return SendClientMessage(playerid, RED, "ล้มเหลว: ราคาขายต้องมากกว่า 0.");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

	if(!IsPlayerInRangeOfPoint(targetid, 10.0, pos[0], pos[1], pos[2])) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 10 เมตรกับผู้เล่นเพื่อใช้คำสั่งนี้.");

	new count = GetOwnedHouseCount(targetid);
	if(GetPlayerVIP(targetid))
	{
		if(count >= MAX_VIP_HOUSES)
		{
			format(string, sizeof(string), "ล้มเหลว: ผู้เล่นคนนั้นเป็นเจ้าของบ้านมากที่สุด x%i หลัง.", MAX_VIP_HOUSES);
			return SendClientMessage(playerid, RED, string);
		}
	}
	else 
	{
		if(count >= MAX_PLAYER_HOUSES)
		{
			format(string, sizeof(string), "ล้มเหลว: ผู้เล่นคนนั้นเป็นเจ้าของบ้านมากที่สุด x%i หลัง.", MAX_PLAYER_HOUSES);
			return SendClientMessage(playerid, RED, string);
		}
	}
	
	TenantData[targetid][tenant_salehouse] = houseid;
	TenantData[targetid][tenant_saleprice] = price;
	TenantData[targetid][tenant_saleowner] = playerid;

	TenantData[targetid][tenant_saleactive] = true;
	
	TenantData[playerid][tenant_saleto] = targetid;
	
	GetPlayerName(targetid, name, sizeof(name));
	format(string, sizeof(string), "ระบบ: คุณได้เสนอบ้านของคุณให้ %s ในราคา $%i. โปรดรอการตอบรับของเขา.", name, price);
	SendClientMessage(playerid, YELLOW, string);
	
	GetPlayerName(playerid, name, sizeof(name));
	format(string, sizeof(string), "ระบบ: %s มีข้อเสนอบ้านให้คุณในราคา $%i พิมพ์ /accepthouse เพื่อยอมรับหรือ /declinehouse เพื่อปฏิเสธ.", name, price);
	return SendClientMessage(targetid, YELLOW, string);
}

YCMD:accepthouse(playerid, params[], help)
{
	new Float:pos[3], query[300], string[128], name[24], houseid = TenantData[playerid][tenant_salehouse], price = TenantData[playerid][tenant_saleprice], targetid = TenantData[playerid][tenant_saleowner];
	if(TenantData[playerid][tenant_saleactive] == false) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณยังไม่ได้รับข้อเสนอใด ๆ ในการซื้อบ้าน.");

	if(targetid == INVALID_PLAYER_ID)
	{
		TenantData[playerid][tenant_saleactive] = false;
		return SendClientMessage(playerid, ORANGE, "ระบบ: เจ้าของบ้านได้ออกจากระบบแล้ว.");
	}
	if(TenantData[targetid][tenant_saleto] != playerid)
	{
		TenantData[playerid][tenant_saleactive] = false;
		return SendClientMessage(playerid, ORANGE, "ระบบ: เจ้าของบ้านได้เสนอบ้านให้คนอื่นแล้ว.");
	}
	if(GetPlayerMoneyEx(playerid) < TenantData[playerid][tenant_saleprice]) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่มีเงินเพียงพอที่จะซื้อบ้านนี้.");

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);

	if(!IsPlayerInRangeOfPoint(targetid, 10.0, pos[0], pos[1], pos[2])) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 10 เมตรกับผู้เล่นเพื่อใช้คำสั่งนี้.");
	
	GivePlayerMoney(playerid, -price);

	GivePlayerMoney(targetid, price);

	HouseData[houseid][house_charid] = GetPlayerCharacterID(playerid);
	
	UpdateNearbyLandValue(houseid);
	
	TenantData[playerid][tenant_saleactive] = false;
	TenantData[targetid][tenant_saleto] = INVALID_PLAYER_ID;

	format(query, sizeof(query), "UPDATE `HOUSES` SET `CHARACTER_ID` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[houseid][house_charid], houseid);
	database_result = db_query@(server_database, query);
	db_free_result@(database_result);

	GetPlayerName(playerid, name, sizeof(name));
	format(string, sizeof(string), "ระบบ: คุณได้ขายบ้านของคุณให้ %s สำเร็จแล้ว.", name);
	SendClientMessage(targetid, YELLOW, string);

	GetPlayerName(targetid, name, sizeof(name));
	format(string, sizeof(string), "ระบบ: คุณได้ซื้อบ้านจาก %s สำเร็จแล้ว.", name);
	SendClientMessage(playerid, YELLOW, string);
	return 1;
}

YCMD:declinehouse(playerid, params[], help)
{
	new Float:pos[3], string[128], name[24], targetid = TenantData[playerid][tenant_saleowner];
	if(TenantData[playerid][tenant_saleactive] == false) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณยังไม่ได้รับข้อเสนอใด ๆ ในการซื้อบ้าน.");
	
	if(targetid == INVALID_PLAYER_ID)
	{
		TenantData[playerid][tenant_saleactive] = false;
		return SendClientMessage(playerid, ORANGE, "ระบบ: เจ้าของบ้านได้ออกจากระบบแล้ว.");
	}
	if(TenantData[targetid][tenant_saleto] != playerid)
	{
		TenantData[playerid][tenant_saleactive] = false;
		return SendClientMessage(playerid, ORANGE, "ระบบ: เจ้าของบ้านได้เสนอบ้านให้คนอื่นแล้ว.");
	}

	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	GetPlayerName(playerid, name, sizeof(name));

	if(!IsPlayerInRangeOfPoint(targetid, 10.0, pos[0], pos[1], pos[2])) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 10 เมตรกับผู้เล่นเพื่อใช้คำสั่งนี้.");

	TenantData[playerid][tenant_saleactive] = false;
	TenantData[targetid][tenant_saleto] = INVALID_PLAYER_ID;

	format(string, sizeof(string), "ระบบ: %s ได้ปฏิเสธข้อเสนอของคุณ", name);
	SendClientMessage(targetid, ORANGE, string);

	GetPlayerName(targetid, name, sizeof(name));
	format(string, sizeof(string), "ระบบ: คุณได้ปฏิเสธข้อเสนอของ %s", name);
	SendClientMessage(playerid, ORANGE, string);
	return 1;
}

//Admin Commands
YCMD:createhouse(playerid, params[], help)
{
	new houseid = GetFreeHouseSlot(), password[64], Float:pos[4], query[600], type[16], city[4];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่มีสิทธิ์ในการใช้คำสั่งนี้.");
	if(sscanf(params, "s[4]s[16]", city, type)) return SendClientMessage(playerid, RED, "วิธีใช้: /createhouse [city] [house1/house2/mansion1/mansion2/apartment]");
	if(houseid == -1) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณมีจำนวนบ้านที่สูงสุดที่เซิร์ฟเวอร์สามารถมีได้แล้ว, ต้องเพิ่ม MAX_HOUSES ในสคริปต์.");

	if(!strcmp(city, "ls", true))
	{
		HouseData[houseid][house_city] = LOS_SANTOS;
	}
	else if(!strcmp(city, "sf", true))
	{
		HouseData[houseid][house_city] = SAN_FIERRO;
	}
	else if(!strcmp(city, "lv", true))
	{
		HouseData[houseid][house_city] = LAS_VENTURAS;
	}
	else return SendClientMessage(playerid, RED, "ล้มเหลว: ไม่มีเมืองที่ถูกต้อง. ต้องเป็น: ls/sf/lv");

	if(!strcmp(type, "house1", true))//1 Story House
	{
		HouseData[houseid][house_value] = (random(500000) + 500000);

		HouseData[houseid][house_intx] = 2196.84;
		HouseData[houseid][house_inty] = -1204.36;
		HouseData[houseid][house_intz] = 1049.02;
		
		HouseData[houseid][house_enterx] = 2193.9001;
		HouseData[houseid][house_entery] = -1202.4185;
		HouseData[houseid][house_enterz] = 1049.0234;
		HouseData[houseid][house_entera] = 91.9386;
		
		HouseData[houseid][house_intinterior] = 6;
		HouseData[houseid][house_intworld] = (houseid + 100);
	}
	else if(!strcmp(type, "house2", true))//2 Story House
	{
  		HouseData[houseid][house_value] = (random(1000000) + 1000000);

		HouseData[houseid][house_intx] = 2317.77;
		HouseData[houseid][house_inty] = -1026.76;
		HouseData[houseid][house_intz] = 1050.21;

		HouseData[houseid][house_enterx] = 2320.0730;
		HouseData[houseid][house_entery] = -1023.9533;
		HouseData[houseid][house_enterz] = 1050.2109;
		HouseData[houseid][house_entera] = 358.4915;

		HouseData[houseid][house_intinterior] = 9;
		HouseData[houseid][house_intworld] = (houseid + 100);
	}
	else if(!strcmp(type, "mansion1", true))//Small Mansion
	{
		HouseData[houseid][house_value] = (random(2000000) + 2000000);

		HouseData[houseid][house_intx] = 2324.41;
		HouseData[houseid][house_inty] = -1149.54;
		HouseData[houseid][house_intz] = 1050.71;

		HouseData[houseid][house_enterx] = 2324.4490;
		HouseData[houseid][house_entery] = -1145.2841;
		HouseData[houseid][house_enterz] = 1050.7101;
		HouseData[houseid][house_entera] = 357.5873;

		HouseData[houseid][house_intinterior] = 12;
		HouseData[houseid][house_intworld] = (houseid + 100);
	}
	else if(!strcmp(type, "mansion2", true))//Large Mansion
	{
		HouseData[houseid][house_value] = (random(4000000) + 4000000);

		HouseData[houseid][house_intx] = 140.28;
		HouseData[houseid][house_inty] = 1365.92;
		HouseData[houseid][house_intz] = 1083.85;

		HouseData[houseid][house_enterx] = 140.1788;
		HouseData[houseid][house_entery] = 1369.1936;
		HouseData[houseid][house_enterz] = 1083.8641;
		HouseData[houseid][house_entera] = 359.2263;

		HouseData[houseid][house_intinterior] = 5;
		HouseData[houseid][house_intworld] = (houseid + 100);
	}
	else if(!strcmp(type, "apartment", true))//Apartment
	{
		HouseData[houseid][house_value] = (random(3000000) + 3000000);

		HouseData[houseid][house_intx] = 225.7121;
		HouseData[houseid][house_inty] = 1021.4438;
		HouseData[houseid][house_intz] = 1084.0177;

		HouseData[houseid][house_enterx] = 225.8993;
		HouseData[houseid][house_entery] = 1023.9148;
		HouseData[houseid][house_enterz] = 1084.0078;
		HouseData[houseid][house_entera] = 358.4921;

		HouseData[houseid][house_intinterior] = 7;
		HouseData[houseid][house_intworld] = (houseid + 100);
	}
	else return SendClientMessage(playerid, RED, "ล้มเหลว: ประเภทบ้านไม่ถูกต้อง. ต้องเป็น: house1/house2/mansion1/mansion2/apartment");
	
	format(password, sizeof(password), "$2y$12$1h2ra6euo5IoIGlVWgvnN.kIOiImlQRnML7Zw/GDZ6Ogb89kA9Lpe");//Randomized Bcrypt Password
	
	GetPlayerPos(playerid, pos[0], pos[1], pos[2]);
	GetPlayerFacingAngle(playerid, pos[3]);
	
	HouseData[houseid][house_charid] = 0;
	HouseData[houseid][house_safe] = 0;
	HouseData[houseid][house_extx] = pos[0];
	HouseData[houseid][house_exty] = pos[1];
	HouseData[houseid][house_extz] = pos[2];
	
	GetXYBehindPlayer(playerid, pos[0], pos[1], 2.0);
	
	HouseData[houseid][house_exitx] = pos[0];
	HouseData[houseid][house_exity] = pos[1];
	HouseData[houseid][house_exitz] = pos[2];
	HouseData[houseid][house_exita] = (pos[3] + 180);
	
	SetPlayerPos(playerid, pos[0], pos[1], pos[2]);

	Iter_Add(Houses, houseid);
	
	HouseData[houseid][house_extinterior] = GetPlayerInterior(playerid);
	HouseData[houseid][house_extworld] = GetPlayerVirtualWorld(playerid);
	
	HouseData[houseid][house_mapicon] = CreateDynamicMapIcon(HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);
	
	HouseData[houseid][house_entercp] = CreateDynamicCP(HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], 1.0, HouseData[houseid][house_extworld], HouseData[houseid][house_extinterior], -1, 4.0);
	HouseData[houseid][house_exitcp] = CreateDynamicCP(HouseData[houseid][house_intx], HouseData[houseid][house_inty], HouseData[houseid][house_intz], 1.0, HouseData[houseid][house_intworld], HouseData[houseid][house_intinterior], -1, 4.0);

	format(query, sizeof(query), "INSERT INTO `HOUSES` (`HOUSE_ID`, `CHARACTER_ID`, `PASS`, `CITY`, `VALUE`, `SAFE`, `EXTX`, `EXTY`, `EXTZ`, `INTX`, `INTY`, `INTZ`, `ENTERX`, `ENTERY`, `ENTERZ`, `ENTERA`, `EXITX`, `EXITY`, `EXITZ`, `EXITA`, `EXTINTERIOR`, `EXTWORLD`, `INTINTERIOR`, `INTWORLD`) VALUES ('%i', '%q', '%s', '%i', '%i', '%i', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%f', '%i', '%i', '%i', '%i')",
	houseid, HouseData[houseid][house_charid], password, HouseData[houseid][house_city], HouseData[houseid][house_value], HouseData[houseid][house_safe], HouseData[houseid][house_extx], HouseData[houseid][house_exty], HouseData[houseid][house_extz], HouseData[houseid][house_intx], HouseData[houseid][house_inty], HouseData[houseid][house_intz], HouseData[houseid][house_enterx], HouseData[houseid][house_entery], 
	HouseData[houseid][house_enterz], HouseData[houseid][house_entera], HouseData[houseid][house_exitx], HouseData[houseid][house_exity], HouseData[houseid][house_exitz], HouseData[houseid][house_exita], HouseData[houseid][house_extinterior], HouseData[houseid][house_extworld], HouseData[houseid][house_intinterior], HouseData[houseid][house_intworld]);
	database_result = db_query@(server_database, query);
	db_free_result@(database_result);
	
	HouseData[houseid][house_active] = true;
	return 1;
}

YCMD:deletehouse(playerid, params[], help)
{
	new query[128];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		    if(IsPlayerInRangeOfPoint(playerid, 5.0, HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz]))
		  	{
				DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
				DestroyDynamicCP(HouseData[i][house_entercp]);
				DestroyDynamicCP(HouseData[i][house_exitcp]);

				Iter_Remove(Houses, i);

				format(query, sizeof(query), "DELETE FROM `HOUSES` WHERE `HOUSE_ID` = '%i'", i);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);

				HouseData[i][house_active] = false;
				return 1;
		    }
	    }
	}
	return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 5 เมตรจากทางเข้าบ้านเพื่อลบบ้าน.");
}

YCMD:deleteallhouses(playerid, params[], help)
{
	new query[128];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		    DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
			DestroyDynamicCP(HouseData[i][house_entercp]);
			DestroyDynamicCP(HouseData[i][house_exitcp]);

			Iter_Remove(Houses, i);

			format(query, sizeof(query), "DELETE FROM `HOUSES` WHERE `HOUSE_ID` = '%i'", i);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);

			HouseData[i][house_active] = false;
	    }
	}
	return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้ลบบ้านทั้งหมดเรียบร้อยแล้ว.");
}

YCMD:removehouseowner(playerid, params[], help)
{
	new password[64], query[400];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		    if(IsPlayerInRangeOfPoint(playerid, 5.0, HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz]))
		  	{
		    	format(password, sizeof(password), "$2y$12$1h2ra6euo5IoIGlVWgvnN.kIOiImlQRnML7Zw/GDZ6Ogb89kA9Lpe");//Randomized Bcrypt Password

		      	HouseData[i][house_charid] = 0;
		     	HouseData[i][house_safe] = 0;

		      	DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
				HouseData[i][house_mapicon] = CreateDynamicMapIcon(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);

				format(query, sizeof(query), "UPDATE `HOUSES` SET `CHARACTER_ID` = '%i', `PASS` = '%s', `SAFE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_charid], password, HouseData[i][house_safe], i);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);

				return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้ลบเจ้าของบ้านเรียบร้อยแล้ว.");
			}
		}
	}
	return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 5 เมตรจากทางเข้าบ้านเพื่อลบเจ้าของบ้าน.");
}

YCMD:removeallhouseowners(playerid, params[], help)
{
	new password[64], query[400];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		  	format(password, sizeof(password), "$2y$12$1h2ra6euo5IoIGlVWgvnN.kIOiImlQRnML7Zw/GDZ6Ogb89kA9Lpe");//Randomized Bcrypt Password

		  	HouseData[i][house_charid] = 0;
		  	HouseData[i][house_safe] = 0;

		 	DestroyDynamicMapIcon(HouseData[i][house_mapicon]);
			HouseData[i][house_mapicon] = CreateDynamicMapIcon(HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz], 31, -1, -1, -1, -1, 250.0, MAPICON_LOCAL, -1, 0);

			format(query, sizeof(query), "UPDATE `HOUSES` SET `CHARACTER_ID` = '%i', `PASS` = '%s', `SAFE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_charid], password, HouseData[i][house_safe], i);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);
		}
	}
	return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้ลบเจ้าของบ้านทั้งหมดเรียบร้อยแล้ว.");
}

YCMD:resethouseprice(playerid, params[], help)
{
	new query[300];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		    if(IsPlayerInRangeOfPoint(playerid, 5.0, HouseData[i][house_extx], HouseData[i][house_exty], HouseData[i][house_extz]))
		  	{
				if(HouseData[i][house_intinterior] == 6)//1 Story House
				{
		        	HouseData[i][house_value] = (random(500000) + 500000);
				}
				else if(HouseData[i][house_intinterior] == 9)//2 Story House
				{
		        	HouseData[i][house_value] = (random(1000000) + 1000000);
				}
				else if(HouseData[i][house_intinterior] == 12)//Small Mansion
				{
		         	HouseData[i][house_value] = (random(2000000) + 2000000);
				}
				else if(HouseData[i][house_intinterior] == 5)//Large Mansion
				{
		         	HouseData[i][house_value] = (random(4000000) + 4000000);
				}
				else if(HouseData[i][house_intinterior] == 7)//Apartment
				{
		       		HouseData[i][house_value] = (random(3000000) + 3000000);
				}

				format(query, sizeof(query), "UPDATE `HOUSES` SET `VALUE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_value], i);
				database_result = db_query@(server_database, query);
				db_free_result@(database_result);

				return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้รีเซ็ตราคาบ้านเรียบร้อยแล้ว.");
		    }
	    }
	}
	return SendClientMessage(playerid, RED, "ล้มเหลว: คุณต้องอยู่ในระยะ 5 เมตรจากทางเข้าบ้านเพื่อรีเซ็ตราคา.");
}

YCMD:resetallhouseprices(playerid, params[], help)
{
	new query[300];
	if(GetPlayerLevel(playerid) != OWNER) return SendClientMessage(playerid, RED, "ล้มเหลว: คุณไม่ใช่เจ้าของเซิร์ฟเวอร์");
	foreach(new i : Houses)
	{
	    if(HouseData[i][house_active] == true)
	    {
		    if(HouseData[i][house_intinterior] == 6)//1 Story House
			{
		     	HouseData[i][house_value] = (random(500000) + 500000);
			}
			else if(HouseData[i][house_intinterior] == 9)//2 Story House
			{
		     	HouseData[i][house_value] = (random(1000000) + 1000000);
			}
			else if(HouseData[i][house_intinterior] == 12)//Small Mansion
			{
		      	HouseData[i][house_value] = (random(2000000) + 2000000);
			}
			else if(HouseData[i][house_intinterior] == 5)//Large Mansion
			{
		      	HouseData[i][house_value] = (random(4000000) + 4000000);
			}
			else if(HouseData[i][house_intinterior] == 7)//Apartment
			{
		     	HouseData[i][house_value] = (random(3000000) + 3000000);
			}

			format(query, sizeof(query), "UPDATE `HOUSES` SET `VALUE` = '%i' WHERE `HOUSE_ID` = '%i'", HouseData[i][house_value], i);
			database_result = db_query@(server_database, query);
			db_free_result@(database_result);
	    }
	}
	return SendClientMessage(playerid, YELLOW, "ระบบ: คุณได้รีเซ็ตราคาบ้านทั้งหมดเรียบร้อยแล้ว.");
}
