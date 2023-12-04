#	$OpenBSD$

PROG=		bindstress
LDADD=		-lpthread
DPADD=		${LIBPTHREAD}
WARNINGS=	yes

run-regress-bindstress:
	${SUDO} ./bindstress

.include <bsd.regress.mk>
