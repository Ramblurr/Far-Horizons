
PRO1 = utils.o parse.o money.o do_prod.o get_ship.o do_est.o do_enemy.o do_dev.o
PRO2 = get_gal.o get_plan.o do_recy.o sav_plan.o do_upg.o do_shipyard.o do_neutral.o
PRO3 = gam_abo.o do_build.o do_res.o get_spnam.o do_hide.o do_ally.o get_transfer.o
PRO4 = get_loc.o do_amb.o get_transact.o sav_transact.o do_int.o get_star.o

PRO_OBJS = Production.o $(PRO1) $(PRO2) $(PRO3) $(PRO4)

Production: $(PRO_OBJS)
	cc $(PRO_OBJS) -no-pie -o ../bin/Production

Production.o: Production.c fh.h
	cc -no-pie -c Production.c

utils.o: utils.c fh.h
	cc -no-pie -c utils.c

parse.o: parse.c fh.h
	cc -no-pie -c parse.c

money.o: money.c fh.h
	cc -no-pie -c money.c

get_gal.o: get_gal.c fh.h
	cc -no-pie -c get_gal.c

get_star.o: get_star.c fh.h
	cc -no-pie -c get_star.c

get_plan.o: get_plan.c fh.h
	cc -no-pie -c get_plan.c

get_ship.o: get_ship.c fh.h
	cc -no-pie -c get_ship.c

get_spnam.o: get_spnam.c fh.h
	cc -no-pie -c get_spnam.c

sav_plan.o: sav_plan.c fh.h
	cc -no-pie -c sav_plan.c

do_prod.o: do_prod.c fh.h
	cc -no-pie -c do_prod.c

do_build.o: do_build.c fh.h
	cc -no-pie -c do_build.c

do_recy.o: do_recy.c fh.h
	cc -no-pie -c do_recy.c

do_res.o: do_res.c fh.h
	cc -no-pie -c do_res.c

do_est.o: do_est.c fh.h
	cc -no-pie -c do_est.c

do_upg.o: do_upg.c fh.h
	cc -no-pie -c do_upg.c

do_amb.o: do_amb.c fh.h
	cc -no-pie -c do_amb.c

do_int.o: do_int.c fh.h
	cc -no-pie -c do_int.c

do_hide.o: do_hide.c fh.h
	cc -no-pie -c do_hide.c

do_ally.o: do_ally.c fh.h
	cc -no-pie -c do_ally.c

do_enemy.o: do_enemy.c fh.h
	cc -no-pie -c do_enemy.c

do_neutral.o: do_neutral.c fh.h
	cc -no-pie -c do_neutral.c

do_shipyard.o: do_shipyard.c fh.h
	cc -no-pie -c do_shipyard.c

do_dev.o: do_dev.c fh.h
	cc -no-pie -c do_dev.c

gam_abo.o: gam_abo.c
	cc -no-pie -c gam_abo.c

get_transact.o: get_transact.c fh.h
	cc -no-pie -c get_transact.c

get_loc.o: get_loc.c fh.h
	cc -no-pie -c get_loc.c

sav_transact.o: sav_transact.c fh.h
	cc -no-pie -c sav_transact.c

get_transfer.o: get_transfer.c fh.h
	cc -no-pie -c get_transfer.c
