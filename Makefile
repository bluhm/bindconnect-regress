#	$OpenBSD$

PROG=		bindstress
LDADD=		-lpthread
DPADD=		${LIBPTHREAD}
WARNINGS=	yes

CLEANFILES=	ktrace.out

${REGRESS_TARGETS}: ${PROG}

REGRESS_TARGETS +=	run-bindstress-default
run-bindstress-default:
	${SUDO} time ${KTRACE} ./bindstress

REGRESS_TARGETS +=	run-bindstress-bind
run-bindstress-bind:
	${SUDO} time ${KTRACE} ./bindstress -n 10 -s 2 -b 5 -c 1

.include <bsd.regress.mk>
