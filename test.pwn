#include "inventory-dialog.inc"

#include <test-boilerplate>
#include <test-boilerplate-items>
#include <zcmd>

main() {
	logger_debug("inventory-dialog", true);
	dbg("inventory-dialog", "debugging logging");
}

static bool:lockInventory;

CMD:lockinv(playerid, params[]) {
	if(lockInventory) {
		lockInventory = false;
		SendClientMessage(playerid, -1, "Inventory unlocked");
	} else {
		lockInventory = true;
		SendClientMessage(playerid, -1, "Inventory locked");
	}

	printViewingState(playerid);
	return 1;
}

new extraListItem1, extraListItem2, extraListItem3;
hook OnPlayerOpenInventory(playerid) {
	if(lockInventory) {
		return Y_HOOKS_BREAK_RETURN_1;
	}

	extraListItem1 = AddInventoryListItem(playerid, "Extra item 1");
	extraListItem2 = AddInventoryListItem(playerid, "Extra item 2");
	extraListItem3 = AddInventoryListItem(playerid, "Extra item 3");

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerSelectExtraItem(playerid, item) {
	new str[128];
	format(str, 128, "Extra item count %d", GetInventoryListItemCount(playerid));
	SendClientMessage(playerid, -1, str);

	if(item == extraListItem1) {
		SendClientMessage(playerid, -1, "Clicked `extraListItem1`, keeping inventory open");
		return Y_HOOKS_BREAK_RETURN_1;
	} else if(item == extraListItem2) {
		SendClientMessage(playerid, -1, "Clicked `extraListItem2`");
	} else if(item == extraListItem3) {
		SendClientMessage(playerid, -1, "Clicked `extraListItem3`");
	} else {
		SendClientMessage(playerid, -1, "Clicked something else, this shouldn't happen!");
	}

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerCloseInventory(playerid) {
	SendClientMessage(playerid, -1, "Closed inventory");

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

new option1, option2, option3;
hook OnPlayerViewInvOpt(playerid) {
	new str[128];
	format(str, 128, "Viewing inventory slot %d", GetPlayerSelectedInventorySlot(playerid));
	SendClientMessage(playerid, -1, str);

	option1 = AddInventoryOption(playerid, "Option 1");
	option2 = AddInventoryOption(playerid, "Option 2");
	option3 = AddInventoryOption(playerid, "Option 3");

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerSelectInvOpt(playerid, option) {
	new str[128];
	format(str, 128, "Option count %d", GetInventoryOptionCount(playerid));
	SendClientMessage(playerid, -1, str);

	if(option == option1) {
		SendClientMessage(playerid, -1, "Clicked `option1`, keeping options open");
		return Y_HOOKS_BREAK_RETURN_1;
	} else if(option == option2) {
		SendClientMessage(playerid, -1, "Clicked `option2`");
	} else if(option == option3) {
		SendClientMessage(playerid, -1, "Clicked `option3`");
	} else {
		SendClientMessage(playerid, -1, "Clicked something else, this shouldn't happen!");
	}

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

hook OnPlayerAddToInventory(playerid, Item:itemid, bool:success) {
	new str[128];
	format(str, 128, "Success: %d: added %d to inventory", success, _:itemid);
	SendClientMessage(playerid, -1, str);

	printViewingState(playerid);
	return Y_HOOKS_CONTINUE_RETURN_0;
}

printViewingState(playerid) {
	new str[128];
	format(str, 128, "Viewing state: %d", IsPlayerViewingInventory(playerid));
	SendClientMessage(playerid, 0xFFFF00FF, str);
}