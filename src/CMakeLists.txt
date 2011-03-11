project(FarHorizons)
cmake_minimum_required(VERSION 2.6)
find_package(LibXml2 REQUIRED)

add_definitions(${LIBXML2_DEFINITIONS})

include_directories(${LIBXML2_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(FH_SRCS
    AddSpeciesAuto.c
    AddSpecies.c
    add_tran.c
    AsciiToBinary.c
    BinaryToAscii.c
    Combat.c
    combat_utils.c
    cons_op.c
    dis_ship.c
    do_all.c
    do_ally.c
    do_amb.c
    do_base.c
    do_bat.c
    do_bomb.c
    do_build.c
    do_deep.c
    do_des.c
    do_dev.c
    do_disband.c
    do_enemy.c
    do_est.c
#     do_friend.c
    do_germ.c
    do_hide.c
    do_inst.c
    do_int.c
    do_jump.c
    do_land.c
    do_locs.c
#     do_man.c
    do_mes.c
#     do_min.c
    do_move.c
    do_name.c
    do_neutral.c
    do_orbit.c
    do_prod.c
    do_recy.c
    do_rep.c
    do_res.c
    do_round.c
    do_scan.c
    do_send.c
    do_shipyard.c
    do_siege.c
    do_teach.c
    do_tech.c
    do_tel.c
    do_terr.c
    do_tran.c
    do_unl.c
    do_upg.c
    do_vis.c
    do_worm.c
    Edit.c
    fight_par.c
    Finish.c
    for_jum.c
    gam_abo.c
    gen_plan.c
    get_gal.c
    get_loc.c
    get_plan.c
    get_ship.c
    get_spnam.c
    get_star.c
    get_transact.c
    get_transfer.c
    HomeSystemAuto.c
    HomeSystem.c
    Jump.c
    ListGalaxy.c
    Locations.c
    MakeHomes.c
    MapGalaxy.c
    MapGalaxyXml.c
    mask.c
    money.c
    Near.c
    NewGalaxy.c
    NoOrders.c
    parse.c
    PostArrival.c
    PreDeparture.c
    Production.c
    regen_sh.c
    Report.c
    sav_plan.c
    sav_star.c
    sav_transact.c
    scan.c
    ScanSpXYZ.c
    Set.c
    ShowGalaxy.c
    Stats.c
    TurnNumber.c
    utils.c
    with_check.c )
    
set(fh_gal get_gal.c get_star.c)
add_executable(MapGalaxyXml ${fh_gal} MapGalaxyXml.c)

target_link_libraries(MapGalaxyXml ${LIBXML2_LIBRARIES})