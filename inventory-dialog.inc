// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_inventory_dialog
	#undef _inc_inventory_dialog
#endif
// custom include-guard to ensure we don't duplicate
#if defined _inventory_dialog_included
	#endinput
#endif
#define _inventory_dialog_included


#include <a_samp>
#include <tick-difference>
#include <item>
#include <inventory>
#include <easyDialog>
#include <YSI\y_hooks>


// -
// Function API
// -


// DisplayPlayerInventory displays the inventory dialog.
forward DisplayPlayerInventory(playerid);

// ClosePlayerInventory closes the inventory dialog, optionally calls
// `OnPlayerCloseInventory`.
forward ClosePlayerInventory(playerid, call = false);

// GetPlayerSelectedInventorySlot returns the selected inventory slot.
forward GetPlayerSelectedInventorySlot(playerid);

// AddInventoryListItem adds a list item to the inventory dialog for additional
// actions. This should only ever be used in `OnPlayerOpenInventory`.
forward AddInventoryListItem(playerid, itemname[]);

// AddInventoryOption adds an inventory option to an item option menu. This
// should only ever be used in `OnPlayerViewInventoryOpt`.
forward AddInventoryOption(playerid, option[]);

// GetInventoryListItems returns the available inventory list items.
forward GetInventoryListItems(playerid, list[], len = sizeof list);

// GetInventoryOptions returns the available inventory options.
forward GetInventoryOptions(playerid, list[], len = sizeof list);

// GetInventoryListItemCount returns the amount of additional inventory list
// items.
forward GetInventoryListItemCount(playerid);

// GetInventoryOptionCount returns the amount of inventory item options.
forward GetInventoryOptionCount(playerid);

// bool returns true if the player is in the inventory screen.
forward bool:IsPlayerViewingInventory(playerid);


// -
// Event API
//

// OnPlayerOpenInventory is called when the player opens their inventory. Return
// 1 to cancel.
forward OnPlayerOpenInventory(playerid);

// OnPlayerCloseInventory is called when a player closes their inventory via
// close button or if `call` is set on `ClosePlayerInventory`.
forward OnPlayerCloseInventory(playerid);

// OnPlayerSelectExtraItem is called when a player selects an inventory list
// item beyond the list of items, an additional option. Return 0 to keep the
// inventory dialog open and 1 to close it.
forward OnPlayerSelectExtraItem(playerid, item);

// OnPlayerViewInventoryOpt is called when the player views the options for an
// item.
forward OnPlayerViewInventoryOpt(playerid);

// OnPlayerSelectInventoryOpt is called when the player selects an option for an
// item. Return 0 to keep the inventory displayed and 1 to close it.
forward OnPlayerSelectInventoryOpt(playerid, option);

// OnPlayerAddToInventory is called when a player adds an item to their
// inventory using a key.
forward OnPlayerAddToInventory(playerid, Item:itemid, bool:success);

// OnPlayerAddedToInventory is called after a player has added an item to their
// inventory.
forward OnPlayerAddedToInventory(playerid, Item:itemid);


// -
// State
// -

static
	inv_ItemListTotal[MAX_PLAYERS],
	inv_SelectedSlot[MAX_PLAYERS],
	bool:inv_ViewingInventory[MAX_PLAYERS],
	inv_ExtraItemList[MAX_PLAYERS][128],
	inv_ExtraItemCount[MAX_PLAYERS],
	inv_OptionsList[MAX_PLAYERS][128],
	inv_OptionsCount[MAX_PLAYERS],
	inv_PutAwayTick[MAX_PLAYERS],
	Timer:inv_PutAwayTimer[MAX_PLAYERS];


// -
// API
// -

stock DisplayPlayerInventory(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	dbg("inventory-dialog", "showing inventory for player",
		_i("playerid", playerid));

	new
		title[18],
		list[(MAX_INVENTORY_SLOTS * (MAX_ITEM_NAME + MAX_ITEM_TEXT + 1)) + 32],
		Item:itemid,
		tmp[5 + MAX_ITEM_NAME + MAX_ITEM_TEXT + 1],
		size;

	inv_ItemListTotal[playerid] = 0;

	new ret = GetPlayerInventorySize(playerid, size);
	if(ret) {
		return 2;
	}

	new itemsize;
	for(new i; i < size; i++) {
		ret = GetInventorySlotItem(playerid, i, itemid);

		if(!IsValidItem(itemid) || ret) {
			break;
		}

		GetItemName(itemid, tmp);

		GetItemTypeSize(GetItemType(itemid), itemsize);
		format(list, sizeof(list), "%s[%02d]%s\n", list, itemsize, tmp);
		inv_ItemListTotal[playerid]++;
	}

	new slots;
	ret = GetInventoryFreeSlots(playerid, slots);

	for(new i; i < slots; i++) {
		strcat(list, "<Empty>\n");
		inv_ItemListTotal[playerid]++;
	}

	inv_ExtraItemList[playerid][0] = EOS;
	inv_ExtraItemCount[playerid] = 0;

	if(CallLocalFunction("OnPlayerOpenInventory", "d", playerid)) {
		return 3;
	}

	if(!isnull(inv_ExtraItemList[playerid])) {
		strcat(list, inv_ExtraItemList[playerid]);
	}

	format(title, sizeof(title), "Inventory (%d/%d)", size - slots, size);

	Dialog_Show(playerid, _inv_inventoryList, DIALOG_STYLE_LIST, title, list, "Options", "Close");

	inv_ViewingInventory[playerid] = true;

	return 0;
}

stock ClosePlayerInventory(playerid, call = false) {
	if(!inv_ViewingInventory[playerid]) {
		return 1;
	}

	dbg("inventory-dialog", "hiding inventory for player",
		_i("playerid", playerid));

	if(call) {
		if(CallLocalFunction("OnPlayerCloseInventory", "d", playerid)) {
			DisplayPlayerInventory(playerid);
			return 2;
		}
	}

	ShowPlayerDialog(playerid, -1, 0, NULL, NULL, NULL, NULL);
	inv_ViewingInventory[playerid] = false;

	return 0;
}

stock GetPlayerSelectedInventorySlot(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return -1;
	}

	return inv_SelectedSlot[playerid];
}

stock AddInventoryListItem(playerid, itemname[]) {
	if(strlen(inv_ExtraItemList[playerid]) + strlen(itemname) > sizeof(inv_ExtraItemList[])) {
		return 0;
	}

	dbg("inventory-dialog", "adding inventory list item",
		_i("playerid", playerid),
		_s("itemname", itemname));

	strcat(inv_ExtraItemList[playerid], itemname);
	strcat(inv_ExtraItemList[playerid], "\n");

	return inv_ExtraItemCount[playerid]++;
}

stock AddInventoryOption(playerid, option[]) {
	if(strlen(inv_OptionsList[playerid]) + strlen(option) > sizeof(inv_OptionsList[])) {
		return 0;
	}

	dbg("inventory-dialog", "adding inventory item option",
		_i("playerid", playerid),
		_s("option", option));

	strcat(inv_OptionsList[playerid], option);
	strcat(inv_OptionsList[playerid], "\n");

	return inv_OptionsCount[playerid]++;
}

stock GetInventoryListItems(playerid, list[], len = sizeof list) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	memcpy(list, inv_ExtraItemList[playerid], 0, 4 * inv_ExtraItemCount[playerid], len);

	return 0;
}

stock GetInventoryOptions(playerid, list[], len = sizeof list) {
	if(!IsPlayerConnected(playerid)) {
		return 1;
	}

	memcpy(list, inv_OptionsList[playerid], 0, 4 * inv_OptionsCount[playerid], len);

	return 0;
}

stock GetInventoryListItemCount(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 0;
	}

	return inv_ExtraItemCount[playerid];
}

stock GetInventoryOptionCount(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return 0;
	}

	return inv_OptionsCount[playerid];
}

stock bool:IsPlayerViewingInventory(playerid) {
	if(!IsPlayerConnected(playerid)) {
		return false;
	}

	return !!inv_ViewingInventory[playerid];
}


// -
// Internal
// -


hook OnScriptInit() {
	for(new i; i < MAX_PLAYERS; i++) {
		for(new j; j < MAX_INVENTORY_SLOTS; j++) {
			inv_SelectedSlot[i] = -1;
		}
	}
}

hook OnPlayerConnect(playerid) {
	for(new j; j < MAX_INVENTORY_SLOTS; j++) {
		inv_SelectedSlot[playerid] = -1;
	}
}

Dialog:_inv_inventoryList(playerid, response, listitem, inputtext[]) {
	dbg("inventory-dialog", "player responded to inventory dialog",
		_i("playerid", playerid),
		_i("response", response),
		_i("listitem", listitem),
		_s("inputtext", inputtext));

	if(!response) {
		ClosePlayerInventory(playerid, true);
		return 1;
	}

	if(listitem >= inv_ItemListTotal[playerid]) {
		if(!CallLocalFunction("OnPlayerSelectExtraItem", "dd", playerid, listitem - inv_ItemListTotal[playerid])) {
			DisplayPlayerInventory(playerid);
		} else {
			dbg("inventory-dialog", "closing inventory via extra item response");
			ClosePlayerInventory(playerid);
		}
		return 1;
	}

	new
		Item:itemid,
		ret;
	ret = GetInventorySlotItem(playerid, listitem, itemid);
	if(ret || !IsValidItem(itemid)) {
		DisplayPlayerInventory(playerid);
	} else {
		inv_SelectedSlot[playerid] = listitem;
		_inv_displaySlotOptions(playerid, listitem);
	}

	return 0;
}

_inv_displaySlotOptions(playerid, slotid) {
	new
		name[MAX_ITEM_NAME + MAX_ITEM_TEXT],
		Item:itemid,
		ret;
	
	ret = GetInventorySlotItem(playerid, slotid, itemid);
	if(ret) {
		return 1;
	}

	GetItemName(itemid, name);
	inv_OptionsList[playerid] = "Equip\nUse\nDrop\n";
	inv_OptionsCount[playerid] = 0;

	CallLocalFunction("OnPlayerViewInventoryOpt", "d", playerid);

	Dialog_Show(playerid, _inv_optionsList, DIALOG_STYLE_LIST, name, inv_OptionsList[playerid], "Accept", "Back");

	return 0;
}

Dialog:_inv_optionsList(playerid, response, listitem, inputtext[]) {
	dbg("inventory-dialog", "player responded to inventory item options",
		_i("playerid", playerid),
		_i("response", response),
		_i("listitem", listitem),
		_s("inputtext", inputtext));

	if(!response) {
		DisplayPlayerInventory(playerid);
		return 1;
	}

	switch(listitem) {
		case 0: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					Item:itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);
				DisplayPlayerInventory(playerid);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		case 1: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					Item:itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);

				PlayerUseItem(playerid);

				ClosePlayerInventory(playerid, true);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		case 2: {
			if(GetPlayerItem(playerid) == INVALID_ITEM_ID) {
				new
					Item:itemid,
					ret;
				
				ret = GetInventorySlotItem(playerid, inv_SelectedSlot[playerid], itemid);
				if(ret) {
					err("failed to get inventory slot item", _i("playerid", playerid), _i("slotid", inv_SelectedSlot[playerid]), _i("ret", ret));
					return 1;
				}

				RemoveItemFromInventory(playerid, inv_SelectedSlot[playerid]);
				GiveWorldItemToPlayer(playerid, itemid, 1);

				PlayerDropItem(playerid);

				ClosePlayerInventory(playerid, true);
			} else {
				ShowActionText(playerid, "You are already holding something", 3000, 200);
				DisplayPlayerInventory(playerid);
			}
		}
		default: {
			if(!CallLocalFunction("OnPlayerSelectInventoryOpt", "dd", playerid, listitem - 3)) {
				DisplayPlayerInventory(playerid);
			} else {
				dbg("inventory-dialog", "closing inventory via option response");
				ClosePlayerInventory(playerid);
			}
		}
	}

	return 1;
}

hook OnPlayerKeyStateChange(playerid, newkeys, oldkeys) {
	if(!IsPlayerInAnyVehicle(playerid)) {
		dbg("inventory-dialog", "OnPlayerKeyStateChange",
			_i("playerid", playerid),
			_i("newkeys", newkeys),
			_i("oldkeys", oldkeys));

		if(newkeys & KEY_CTRL_BACK) {
			DisplayPlayerInventory(playerid);
		} else if(newkeys & KEY_YES) {
			_inv_putAway(playerid);
		}
	}

	return 1;
}

_inv_putAway(playerid) {
	if(GetTickCountDifference(GetTickCount(), inv_PutAwayTick[playerid]) < 1000) {
		return;
	}

	new Item:itemid = GetPlayerItem(playerid);

	if(!IsValidItem(itemid)) {
		return;
	}

	new
		itemsize,
		freeslots;

	GetItemTypeSize(GetItemType(itemid), itemsize);
	
	new ret = GetInventoryFreeSlots(playerid, freeslots);
	if(ret) {
		err("failed to get inventory free slots",
			_i("return", ret));
		return;
	}

	if(itemsize > freeslots) {
		CallLocalFunction("OnPlayerAddToInventory", "ddd", playerid, _:itemid, 0);
		return;
	}

	if(CallLocalFunction("OnPlayerAddToInventory", "ddd", playerid, _:itemid, 1)) {
		return;
	}

	inv_PutAwayTick[playerid] = GetTickCount();

	ApplyAnimation(playerid, "PED", "PHONE_IN", 4.0, 1, 0, 0, 0, 300);
	stop inv_PutAwayTimer[playerid];
	inv_PutAwayTimer[playerid] = defer PlayerPutItemInInventory(playerid, _:itemid);

	return;
}

timer PlayerPutItemInInventory[300](playerid, itemid) {
	AddItemToInventory(playerid, Item:itemid);
	CallLocalFunction("OnPlayerAddedToInventory", "dd", playerid, itemid);
}
