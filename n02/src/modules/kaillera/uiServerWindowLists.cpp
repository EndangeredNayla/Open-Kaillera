#include "uiServerWindowLists.h"
#include "modKaillera.h"
#include "module.h"
#include "common.h"
#include "clientgui.h"
#include "juceKailleraServerConnection.h"
#include "juceKailleraServerGame.h"
#include "gameSelect.h"
#include "kaillera_ClientCore.h"


namespace n02 {

	namespace kaillera {

		/************************************************************
		** vars
		*******************************/

		extern unsigned short lastSelectedUserId;
		extern char lastGame[128];

		bool isGameWindowActive();

		/************************************************************
		** 
		*******************************/
		juce::tchar * connectionSettingTexts [] = {
			0,
			T("LAN"),
			T("Exc"),
			T("Good"),
			T("Avg"),
			T("Bad"),
			T("Low")
		};

		juce::tchar * userStatusTexts [] = {
			T("Conneting"),
			T("Idle"),
			T("Playing")
		};

		juce::tchar * gameStatusTexts [] = {
			T("Waiting"),
			T("Playing"),
			T("Playing")
		};


		// players list
		DynamicOrderedArray<KailleraPlayerT, 8> players;

		// players list drawing
		int  KailleraPlayersList::getNumRows ()
		{
			TRACE();
			return players.itemsCount();
		}
		void  KailleraPlayersList::paintRowBackground (Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
		{
			TRACE();
			if (rowIsSelected)
				g.fillAll (Colour(0xdd,0xdd,0xff));
			TRACE();
		}
		void  KailleraPlayersList::paintCell (Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
		{
			TRACE();
			g.setColour (Colours::black);
			if (rowNumber >= 0 && rowNumber < players.itemsCount()) {
				KailleraPlayerT & k = players[rowNumber];
				switch (columnId) {
					case 1:
						g.drawText (k.name, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 2:
						{
							String text(k.ping);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
					case 3:
						g.drawText (connectionSettingTexts[k.connectionSetting], 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 4:
						{
							String text(((((k.ping  * 60) / (1000 * k.connectionSetting))+2) * k.connectionSetting) -1);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
				};
			}
			g.setColour (Colours::black.withAlpha (0.2f));
			g.fillRect (width - 1, 0, 1, height);
			TRACE();
		}
		void  KailleraPlayersList::cellClicked (int rowNumber, int columnId, const MouseEvent &e) {
			if (rowNumber > 0 && rowNumber < players.itemsCount()) {
				lastSelectedUserId = players.getItemPtr(rowNumber)->id;
			} else {
				lastSelectedUserId = 0xffff;
			}
		}
		



		/************************************************************
		** users list
		*******************************/

		DynamicOrderedArray<KailleraUserT, 100> users;

		// list drawing
		int   KailleraUsers::getNumRows ()
		{
			TRACE();
			return users.itemsCount();
		}
		void  KailleraUsers::paintRowBackground (Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
		{
			TRACE();
			if (rowIsSelected)
				g.fillAll (Colour(0xdd,0xdd,0xff));
			TRACE();
		}
		void  KailleraUsers::paintCell (Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
		{
			TRACE();
			g.setColour (Colours::black);

			if (rowNumber >= 0 && rowNumber < users.itemsCount()) {
				KailleraUserT k = users[rowNumber];
				switch (columnId) {
					case 1:
						g.drawText (k.username, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 2:
						{
							String text(k.ping);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
					case 3:
						{
							String text(connectionSettingTexts[k.connectionSetting]);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
					case 4:
						{
							String text(userStatusTexts[k.status]);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
				};
			}
			g.setColour (Colours::black.withAlpha (0.2f));
			g.fillRect (width - 1, 0, 1, height);
			TRACE();
		}
		void  KailleraUsers::cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e)
		{
			
		}

		/************************************************************
		** games list
		*******************************/

		DynamicOrderedArray<KailleraGameT, 50> games;

		// list drawing
		int   KailleraGames::getNumRows ()
		{
			return games.itemsCount();
		}
		void  KailleraGames::paintRowBackground (Graphics &g, int rowNumber, int width, int height, bool rowIsSelected)
		{
			if (rowIsSelected)
				g.fillAll (Colour(0xdd,0xdd,0xff));
		}
		void  KailleraGames::paintCell (Graphics &g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
		{
			g.setColour (Colours::black);

			if (rowNumber >= 0 && rowNumber < games.itemsCount()) {
				KailleraGameT & k = games[rowNumber];
				switch (columnId) {
					case 1:
						g.drawText (k.name, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 2:
						g.drawText (k.app, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 3:
						g.drawText (k.owner, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
					case 4:
						{
							String text(gameStatusTexts[k.status]);
							g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
						}
						break;
					case 5:
						g.drawText (k.players, 2, 0, width - 4, height, Justification::centredLeft, true);
						break;
				};
			}
			g.setColour (Colours::black.withAlpha (0.2f));
			g.fillRect (width - 1, 0, 1, height);
		}

		void  KailleraGames::cellDoubleClicked (int rowNumber, int columnId, const MouseEvent &e)
		{
			if (!isGameWindowActive() && rowNumber >= 0 && rowNumber < games.itemsCount()) {
				KailleraGameT & game = games[rowNumber];
				if (game.status == 0) {
					strcpy(lastGame, game.name);
					uiGameJoinCallback(game.id);
				}
			}
		}


		void processCommand(KailleraListsCommand * cmd) {
			switch(cmd->command) {
				case LISTCMD_ADDGAME:
					{
						KailleraGameT * g = cmd->body.game;
						games.addItemPtr(g);
						delete g;
					}
					break;
				case LISTCMD_REMGAME:
					{
						KailleraGameT * game = cmd->body.game;

						for (int x = 0; x < games.itemsCount(); x++) {
							register KailleraGameT * g = games.getItemPtr(x);
							if (game->id == g->id) {
								games.removeIndex(x);
								break;
							}
						}
						delete game;
					}
					break;


				case LISTCMD_STATGAME:
					{
						KailleraGameT * game = cmd->body.game;

						for (int x = 0; x < games.itemsCount(); x++) {
							register KailleraGameT * g = games.getItemPtr(x);
							if (game->id == g->id) {
								strcpy(g->players, game->players);
								g->status = game->status;
								break;
							}
						}
						delete game;
					}
					break;

				case LISTCMD_REMALLGAMES:
					games.clearItems();
					break;

				case LISTCMD_ADDUSER:
					{
						KailleraUserT * u = cmd->body.user;
						users.addItemPtr(u);
						delete u;
					}
					break;
				case LISTCMD_REMUSER:
					for (int x = 0; x < users.itemsCount(); x++) {
						register KailleraUserT * u = users.getItemPtr(x);
						if (cmd->body.user->id == u->id) {
							users.removeIndex(x);
							break;
						}
					}
					delete cmd->body.user;
					break;

				case LISTCMD_REMALLUSERS:
					users.clearItems();
					break;

				case LISTCMD_ADDPLAYER:
					{
						KailleraPlayerT * p = cmd->body.player;
						players.addItemPtr(p);
						delete p;
					}
					break;

				case LISTCMD_REMPLAYER:
					{
						for (int x = 0; x < players.itemsCount(); x++) {
							register KailleraPlayerT * p = players.getItemPtr(x);
							if (cmd->body.player->id == p->id) {
								players.removeIndex(x);
								break;
							}
						}
						delete cmd->body.player;
					}
					break;

				case LISTCMD_REMALLPLAYERS:
					players.clearItems();
					break;
			};
			delete cmd;
		}
	};
};
