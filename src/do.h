// Far Horizons Game Engine
// Copyright (C) 2022 Michael D Henderson
// Copyright (C) 2021 Raven Zachary
// Copyright (C) 2019 Casey Link, Adam Piggott
// Copyright (C) 1999 Richard A. Morneau
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#ifndef FAR_HORIZONS_DO_H
#define FAR_HORIZONS_DO_H

void do_AMBUSH_command(void);
void do_ALLY_command(void);
void do_BASE_command(void);
void do_BUILD_command(int continuing_construction, int interspecies_construction);
void do_DEEP_command(void);
void do_DESTROY_command(void);
void do_DEVELOP_command(void);
void do_DISBAND_command(void);
void do_ENEMY_command(void);
void do_ESTIMATE_command(void);
void do_HIDE_command(void);
void do_INSTALL_command(void);
void do_INTERCEPT_command(void);
void do_LAND_command(void);
void do_JUMP_command(int jumped_in_combat, int using_jump_portal);
void do_MESSAGE_command(void);
void do_MOVE_command(void);
void do_NAME_command(void);
void do_NEUTRAL_command(void);
void do_ORBIT_command(void);
void do_PRODUCTION_command(int missing_production_order);
void do_RECYCLE_command(void);
void do_REPAIR_command(void);
void do_RESEARCH_command(void);
void do_SCAN_command(void);
void do_SEND_command(void);
void do_SHIPYARD_command(void);
void do_TRANSFER_command(void);
void do_UNLOAD_command(void);
void do_UPGRADE_command(void);
void do_VISITED_command(void);
void do_WORMHOLE_command(void);

#endif //FAR_HORIZONS_DO_H
