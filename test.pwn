#include "inventory-dialog.inc"

#include <test-boilerplate>

main() {
	logger_debug("inventory-dialog", true);
	dbg("inventory-dialog", "debugging logging");
	new s[1];
	DisplayPlayerInventory(0);
	ClosePlayerInventory(0);
	GetPlayerSelectedInventorySlot(0);
	AddInventoryListItem(0, s);
	AddInventoryOption(0, s);
	GetInventoryListItems(0, s, 0);
	GetInventoryOptions(0, s, 0);
	GetInventoryListItemCount(0);
	GetInventoryOptionCount(0);
	IsPlayerViewingInventory(0);
}
