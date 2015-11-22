import pylab as plt
import yodapy as yp
import math
import operator
import scipy
import matplotlib.font_manager as pltf

#def getMFIsquarePropModel(bclust=2.2, rho=.59, pim=0.0, kmaxFP=10**6, nMax=100, fpmodel=1):
def getMFIsquarePropModel(bclust=2.2, rho=.59, pim=0.0, kmaxFP=10**6, nMax=100, fpmodel=1, beta=1.):
	#
	# beta: if fpmodel=3, we have a beta exponent as a shape parameter: k=alpha*n^beta. we can use 4*rho_0 for alpha i guess.\
	# of course, after the first step, each new element can only have 3 new neighbors, so 3*rho_0*n^beta?
	# expected values for integer n for square prop. model.
	# Nf has two terms. the first term is 3 factors: P(goes out), P(hasn't gone out yet) which is a PI funct., sum/integral over all clusters k>k
	# the integral might be treated explicitly or as b~1, so we get (kmax-k)/(1-b) or ln(kmax/k)
	# the second term is prob-survives (product function) * prob of hitting a k' cluster (see paper).
	if pim>16.0:
			print "screwy value of pim."
			return None
	kmax=kmaxFP*rho
	prod1=1.0
	prod2=1.0
	output=[]	# n, kFootPrint, <k>, N1, N2
	print "fpmodel: %s" % str(fpmodel)
	for n in xrange(1,nMax):
		if fpmodel==0 or fpmodel==None: footprint=(2*n+1)**2	# square NNN propagation
		if fpmodel==1: footprint=(1+2*(n*n + n))	# "square-circle" NN propagation; C=4n
		if fpmodel==2: footprint=(1+4*n**beta)		# this seems to be the best model. we don't need to "fix" small n for neg. log problems, etc. we get good results.
		if fpmodel==3:
			# this gives more dramatic results for beta<1, but introduces computational problems fo k<5 (or whatever), so fitting is tricky.
			# can we call this a branching ratio? aka, is pi a branching ratio?
			footprint=(1+(3.14**(beta-1.))*n**beta)
			if footprint<5.: footprint=5.
		#
		kf=rho*footprint
		#kfPrev=rho*(2*n-1)**2 #(2*(n-1)+1)**2
		#kfPrev=rho*(1+2*(n*n-n))	#1 + 2*((n-1)*(n-1)+(n-1))	# but we don't use this variable any more.
		#
		# notation: x-> term; {1,2,3} first, second, etc. term; {a,b} first, second, etc. product in term; {1,2,..} first, second, etc. version
		x1a=pim/kf	# prob it goes out at this kf
		x1b=1.0-(pim/kf)	# note: this factore does not get included until the next iteration (note when prod1, prod2 are updated))
		#if x1b<0: x1b=0
		if x1b<0: x1b=1.0-(pim/footprint)	# assume that density will fluctuate...
		#x1b=1.0-pim/float(kfPrev)	# prob it survived to last kf where immunity was evaluated.
		#
		if kf>kmax: break
		x1c1=(kmax**(2.0-bclust)-kf**(2.0-bclust))/(2.0-bclust)	# sum/integral of clusters where k' fire can occur (cluster selected)
		x1c2=math.log(kmax/kf)												# ln limit of # sum/integral of clusters where k' fire can occur. (b~1)
		#
		#
		x2a=kf**(1.0-bclust)		# prob cluster size kf is selected
		x2b=1.0-(pim/kf)	# note, for this geometry, x1b=x2b; contribution to product of prob that fire survived through last immunity eval.
		if x2b<0: x2b=0
		#
		# now, compute total value using b=1.0 (ln() solution) and b=1.2:
		#
		# note: in the first term, we want the probability that the fire went out at the previous step
		#N1=x1a*(x1b*prod1)*x1c1 + x2a*x2b*prod2
		#N2=x1a*(x1b*prod1)*x1c2 + x2a*x2b*prod2
		N1=x1a*(prod1)*x1c1 + x2a*prod2
		N2=x1a*(prod1)*x1c2 + x2a*prod2
		#
		prod1*=x1b
		prod2*=x2b
		#
		#print x1a, x1b, x1c1, x1c2, x2a, x2b, pim/kf, prod1, prod2
		output+=[[n, footprint, kf, N1, N2]]
	return output

def plotMFIdistsRho(bclust=2.2, rhorange=[.1, 1.0, .2], kmaxFP=10**6, nMax=200, pim=4.9, fpmodel=1, fignums=[0,1]):
	# rhorange = [minrho, maxrho, dRho]
	bofpim=[]
	for fignum in fignums:
		plt.figure(fignum)
		plt.clf()
		plt.xlabel('$log_{10}(k)$', size=18)
		plt.ylabel('$log_{10}(N(k))$', size=18)
		
	#
	primeRhos=[.1, .3, .5, .7, .9, 1.0]
	thisrho=rhorange[0]
	icount=0
	while thisrho<=rhorange[1]:
		X=getMFIsquarePropModel(bclust, thisrho, pim, kmaxFP, nMax, fpmodel)
		x=[]
		y1=[]
		y2=[]
		xstart=25
		for rw in X:
			#print rw
			x+=[math.log10(rw[2])]
			#print rw[3]
			y1+=[math.log10(rw[3])]
			y2+=[math.log10(rw[4])]
		lf1=yp.linefit([x[xstart:],y1[xstart:]])
		lf2=yp.linefit([x[xstart:],y2[xstart:]])
		lf1.doFit()
		lf2.doFit()
		#lf1.doFit(None, None, 3)
		#lf2.doFit(None, None, 3)
		#
		bofpim+=[[pim, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		plt.figure(fignums[0])
		plt.title('$\\beta=2.2$')
		plt.plot(x,y1,'%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, b=%s$' % (str(thisrho), str(pim), str(-lf1.b)), lw=2)		# beta!=2
		plt.plot([x[0], x[xstart], x[-1]], [lf1.a +lf1.b*x[0], lf1.a +lf1.b*x[xstart], lf1.a +lf1.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=15)
		plt.figure(fignums[1])
		plt.title('$\\beta=2.0$')
		plt.plot(x, y2, '%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, b=%s$' % (str(thisrho), str(pim), str(-lf2.b)), lw=2)		# beta=2 approximation
		plt.plot([x[0], x[xstart], x[-1]], [lf2.a +lf2.b*x[0], lf2.a +lf2.b*x[xstart], lf2.a +lf2.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=15)
		icount+=1
		#
		thisrho+=rhorange[2]
	plt.show()
	#
	for fignum in fignums:
		plt.figure(fignum)
		plt.legend(loc='best')
	return bofpim
	
def plotMFIdists(bclust=2.2, rho=.59, kmaxFP=10**6, nMax=200, pimrange=[0.0, 4.9], doClf=[True, True, False], fpmodel=1, lineNum=0):
	#pim=0.0
	# lineNum is the index of the line on the plot when we plot multiple lines on a single canvas.
	beta=1.5
	print "lineNum: %d" % lineNum
	xstart=15
	pim=pimrange[0]
	bofpim=[]
	plt.figure(0)
	if doClf[0]: plt.clf()
	plt.figure(1)
	if doClf[1]: plt.clf()
	
	plt.figure(3)
	plt.clf()
	plt.figure(4)
	plt.clf()
	#primePims=[0.0, .4, 1.0, 1.4, 2.0, 2.4, 3.0, 3.4, 4.0, 4.6, 4.8]
	#primePims=[0, 4, 10, 14, 20, 24, 30, 34, 40, 46, 49]
	primePims=[0, 10, 20, 30, 40, 48]
	
	icount=-1
	icount2=0
	while pim<=(pimrange[1]+.0001):
		print pim
		icount+=1
		X=getMFIsquarePropModel(bclust, rho, pim, kmaxFP, nMax, fpmodel, beta)
		x=[]
		y1=[]
		y2=[]
		for rw in X:
			#print rw
			x+=[math.log10(rw[2])]
			#print rw[3]
			y1+=[math.log10(rw[3])]
			y2+=[math.log10(rw[4])]
		lf1=yp.linefit([x[xstart:],y1[xstart:]])
		lf2=yp.linefit([x[xstart:],y2[xstart:]])
		lf1.doFit()
		lf2.doFit()
		#lf1.doFit(None, None, 3)
		#lf2.doFit(None, None, 3)
		#
		bofpim+=[[pim, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		a1=bofpim[-1][1]
		b1=bofpim[-1][2]
		a2=bofpim[-1][4]
		b2=bofpim[-1][5]
		
		plt.figure(0)
		#plt.loglog(map(operator.itemgetter(2), X), map(operator.itemgetter(3), X), '.-')
		plt.plot(x,y1,'%s.-' % yp.pycolor(icount), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-lf1.b)))		# beta!=2 
		#plt.plot([x[0], x[-1]], [a1-b1*x[0], a1-b1*x[-1]], '%s*-' % yp.pycolor(icount))
		#
		plt.figure(1)
		#plt.loglog(map(operator.itemgetter(2), X), map(operator.itemgetter(4), X), '.-')
		plt.plot(x, y2, '%s.-' % yp.pycolor(icount), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-lf2.b)))		# beta=2 approximation
		#plt.plot([x[0], x[-1]], [a2-b2*x[0], a2-b2*x[-1]], '%s*-' % yp.pycolor(icount))
		#
		#plt.loglog(map(operator.itemgetter(2), X), map(operator.itemgetter(3), X), '.-')
		if int(10.0*pim) in primePims:
			plt.figure(3)
			plt.plot(x,y1,'%s.-' % yp.pycolor(icount2), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-round(lf1.b,2))), lw=2)
			plt.plot([x[0], x[xstart], x[-1]], [a1-b1*x[0], a1-b1*x[xstart], a1-b1*x[-1]], '%s*-' % yp.pycolor(icount2))
			plt.figure(4)
			plt.plot(x,y2,'%s.-' % yp.pycolor(icount2), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-round(lf2.b,2))), lw=2)
			plt.plot([x[0], x[xstart], x[-1]], [a2-b2*x[0], a2-b2*x[xstart], a2-b2*x[-1]], '%s*-' % yp.pycolor(icount2))
			icount2+=1
		pim+=0.2
		
	
		plt.figure(0)
	plt.legend(loc='upper right')
	plt.figure(1)
	plt.legend(loc='upper right')
	plt.figure(3)
	plt.legend(loc='lower left')
	plt.xlabel('$log(k)$', size=18)
	plt.ylabel('$log(N(k))$', size=18)
	plt.figure(4)
	plt.legend(loc='lower left')
	plt.xlabel('$log(k)$', size=18)
	plt.ylabel('$log(N(k))$', size=18)
	for i in xrange(5):
		plt.figure(i)
		plt.title('$\\rho=%f$' % rho)
	
	plt.figure(2)
	if doClf[2]: plt.clf()
	#if int(10.0*pim) in primePims:
	plt.plot(map(operator.itemgetter(0), bofpim), map(operator.itemgetter(2), bofpim), '%s.-' % yp.pycolor(lineNum), label='$ \\beta = 2.2, \\rho = %s $' % str(rho), lw=2)
	plt.plot(map(operator.itemgetter(0), bofpim), map(operator.itemgetter(5), bofpim), '%s>-' % yp.pycolor(lineNum), label='$ \\beta = 2.0,  \\rho = %s $' % str(rho), lw=2)
	plt.xlabel('$\\epsilon$', size=18)
	plt.ylabel('$b_{fires}$', size=18)
	plt.show()
	#
	return bofpim

def doFig4(bclust=2.2, kmaxFP=10**9, nMax=200, pim=4.9, betas=[.9, 1.0, 1.25, 1.5, 1.75, 2.0, 2.5], fignums=[0,1]):
	return plotMFIdistsBeta(bclust, kmaxFP, nMax, pim, betas, fignums, True)
	
#def plotMFIdistsBeta(bclust=2.2, rhorange=[.1, 1.0, .2], kmaxFP=10**6, nMax=200, pim=4.9, fpmodel=1, fignums=[0,1]):
def plotMFIdistsBeta(bclust=2.2, kmaxFP=10**9, nMax=200, pim=4.9, betas=[.9, 1.0, 1.25, 1.5, 1.75, 2.0, 2.5], fignums=[0,1], dosave=False):
	# "Fig-4" in PRE publication.
	#
	# "beta" version, use footprint ~ n**beta 1<beta<2.0 (or maybe 2.25 or something).
	# for now, fixed epsilon (pim), fixed rho.
	# rhorange = [minrho, maxrho, dRho]
	# note: bcluse is slope of b-distribution, beta is the exponent for shape area/raduis (n) scaling.
	fpmodel=2
	rho=1.0	# basically, this puts all the empty sites into the fractal/branching structure.
	thisrho=rho
	#
	fsize=20
	thisfont=pltf.FontProperties(size=fsize);
	thisfont2=pltf.FontProperties(size=fsize-2);
	#
	bofpim=[]
	for fignum in fignums:
		plt.figure(fignum)
		plt.clf()
		plt.xlabel('$log_{10}(k)$', size=fsize+2)
		plt.ylabel('$log_{10}(N(k))$', size=fsize+2)
	#
	#starts=[]
	#
	primeRhos=[.1, .3, .5, .7, .9, 1.0]
	#thisrho=rhorange[0]
	icount=0
	#while thisrho<=rhorange[1]:
	fitMarkerShortList=['o', '^', 's', 'p', '*', 'h', '+', 'H', 'D', 'x']
	for beta in betas:
		#X=getMFIsquarePropModel(bclust, thisrho, pim, kmaxFP, nMax, fpmodel)
		print "prams: %s, %s, %s, %s, %s, %s, %s" % (bclust, thisrho, pim, kmaxFP, nMax, fpmodel, beta)
		X=getMFIsquarePropModel(bclust, thisrho, pim, kmaxFP, nMax, fpmodel, beta)
		print "beta=%f, len=%d" % (beta, len(X))
		x=[]
		y1=[]
		y2=[]
		#xstart=25
		xstart=2
		sxtop=500
		for rw in X:
			#print rw
			x+=[math.log10(rw[2])]
			#print rw[3]
			y1+=[math.log10(rw[3])]
			y2+=[math.log10(rw[4])]
		lf1=yp.linefit([x[xstart:],y1[xstart:]])
		lf2=yp.linefit([x[xstart:],y2[xstart:]])
		lf1.doFit()
		lf2.doFit()
		#lf1.doFit(None, None, 3)
		#lf2.doFit(None, None, 3)
		#
		if fpmodel>1:
			bofpim+=[[pim, beta, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		else:
			bofpim+=[[pim, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		plt.figure(fignums[0])
		#plt.title('$\\beta=2.2$')
		#plt.plot(x[1:],y1[1:],'%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, \\beta_s=%s, b=%s$' % (str(thisrho), str(pim), str(beta), str(-lf1.b)[0:5]), lw=2)		# beta!=2
		dotsies=yp.integerSpacedPoints([x[1:],y1[1:]], 0.5)
		dotsies[0]+=[x[-1]]
		dotsies[1]+=[y1[-1]]	# add end-bits for completeness.
		plt.plot(x[1:],y1[1:],'%s-' % yp.pycolor(icount), lw=2, ms=10)		# beta!=2
		plt.plot(dotsies[0], dotsies[1], '%s%s' % (yp.pycolor(icount), fitMarkerShortList[icount%len(fitMarkerShortList)]), label='$D=%s, b=%s$' % (str(beta), str(-lf1.b)[0:5]), ms=10 )
		plt.plot([x[0], x[xstart], x[-1]], [lf1.a +lf1.b*x[0], lf1.a +lf1.b*x[xstart], lf1.a +lf1.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=20, lw=2)
		#
		plt.figure(fignums[1])
		#plt.title('$\\beta=2.0$')
		#plt.plot(x[1:], y2[1:], '%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, \\beta_s=%s, b=%s$' % (str(thisrho), str(pim), str(beta), str(-lf2.b)[0:5]), lw=2)		# beta=2 approximation
		plt.plot(x[1:], y2[1:], '%s.-' % yp.pycolor(icount), label='$D=%s, b=%s$' % (str(beta), str(-lf2.b)[0:5]), lw=2, ms=10)		# beta=2 approximation
		plt.plot([x[0], x[xstart], x[-1]], [lf2.a +lf2.b*x[0], lf2.a +lf2.b*x[xstart], lf2.a +lf2.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=15, lw=2)
		icount+=1
		#
		#thisrho+=rhorange[2]
	#
	for fignum in fignums:
		plt.figure(fignum)
		ax=plt.gca()
		plt.subplots_adjust(bottom=.15)
		plt.setp(ax.get_xticklabels(), fontsize=fsize)
		plt.setp(ax.get_yticklabels(), fontsize=fsize)
		
		#plt.legend(loc='best', numpoints=1, prop=thisfont2, ncol=1, title='$\\epsilon=4.9$')
		lgd=plt.legend(loc='best', numpoints=1, prop=thisfont2, ncol=1)
		lgd.set_title('$\\epsilon=4.9$')
		plt.setp(lgd.get_title(), fontsize=fsize)
		# mfi-fractalType-varEps-bc22-D125
		if fignum>0: continue
		if dosave:
			plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/mfi-fractalType-eps49-bc%s-varD.png' % str(bclust).replace('.', ''))
			plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/mfi-fractalType-eps49-bc%s-varD.eps' % str(bclust).replace('.', ''))
	plt.show()
	return bofpim

def doFig5(bclust=2.2, kmaxFP=10**9, nMax=200, pimrange=[0.0,5.0,1.], beta=1.25, fignums=[0,1]):
	return plotMFIdistsBetaEps(bclust, kmaxFP, nMax, pimrange, beta, fignums)

def plotMFIdistsBetaEps(bclust=2.2, kmaxFP=10**9, nMax=200, pimrange=[0.0,5.0,1.], beta=1.25, fignums=[0,1]):
#def plotMFIdistsBetaEps(bclust=2.2, kmaxFP=10**9, nMax=200, pimlist=[0.0,1.0, 2.0, 3.0, 4.0, 4.9], beta=1.25, fignums=[0,1]):
	# (this function is probably redundant. same as ...Beta(), but fix beta and range over pim.) we know we get nice PL
	# when we vary the shape dimension. what about if we fix D (beta) and vary eps. with the fractal-footprint?
	#
	# fig 5 in PRE pub.
	#
	#pimrange=[0.0,4.9,1.]
	# "beta" version, use footprint ~ n**beta 1<beta<2.0 (or maybe 2.25 or something).
	# for now, fixed epsilon (pim), fixed rho.
	# rhorange = [minrho, maxrho, dRho]
	# note: bcluse is slope of b-distribution, beta is the exponent for shape area/raduis (n) scaling.
	fsize=20
	thisfont=pltf.FontProperties(size=fsize);
	thisfont2=pltf.FontProperties(size=fsize-2);
	#
	fpmodel=2	# aka, MFI with fractal dimension treatment (as opposed to solid shapes, etc.)
	rho=1.0	# basically, this puts all the empty sites into the fractal/branching structure.
	thisrho=rho
	bofpim=[]
	for fignum in fignums:
		plt.figure(fignum)
		plt.clf()
		plt.xlabel('$log_{10}(k)$', size=fsize+2)
		plt.ylabel('$log_{10}(N(k))$', size=fsize+2)
	#
	#starts=[]
	#
	primeRhos=[.1, .3, .5, .7, .9, 1.0]
	#thisrho=rhorange[0]
	icount=0
	#while thisrho<=rhorange[1]:
	#for beta in betas:
	pim=pimrange[0]
	pim0=pim
	while pim0<=pimrange[1]:
	#for pim in pimlist:
		#X=getMFIsquarePropModel(bclust, thisrho, pim, kmaxFP, nMax, fpmodel)
		pim=pim0
		if pim>4.9: pim=4.9
		print "prams: %s, %s, %s, %s, %s, %s, %s" % (bclust, thisrho, pim, kmaxFP, nMax, fpmodel, beta)
		#pim=pimrange[0]+float(icount)*pimrange[2]
		X=getMFIsquarePropModel(bclust, thisrho, pim, kmaxFP, nMax, fpmodel, beta)
		print "beta=%f, len=%d" % (beta, len(X))
		x=[]
		y1=[]
		y2=[]
		#xstart=25
		xstart=15
		sxtop=500
		for rw in X:
			#print rw
			x+=[math.log10(rw[2])]
			#print rw[3]
			y1+=[math.log10(rw[3])]
			y2+=[math.log10(rw[4])]
		lf1=yp.linefit([x[xstart:],y1[xstart:]])
		lf2=yp.linefit([x[xstart:],y2[xstart:]])
		lf1.doFit()
		lf2.doFit()
		#lf1.doFit(None, None, 3)
		#lf2.doFit(None, None, 3)
		#
		dotsies=yp.integerSpacedPoints([x[1:],y1[1:]], .5)
		dotsies[0]+=[x[-1]]
		dotsies[1]+=[y1[-1]]
		if fpmodel>1:
			bofpim+=[[pim, beta, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		else:
			bofpim+=[[pim, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		plt.figure(fignums[0])
		#plt.title('$\\beta=2.2$')
		#plt.plot(x[1:],y1[1:],'%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, D=%s, b=%s$' % (str(thisrho), str(pim), str(beta), str(-lf1.b)[0:5]), lw=2)		# beta!=2
		plt.plot(dotsies[0], dotsies[1], '%s%s'  % (yp.pycolor(icount), yp.fitMarkerShortList[icount%len(yp.fitMarkerShortList)]), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-lf1.b)[0:5]),  ms=10)
		plt.plot(x[1:],y1[1:],'%s-' % yp.pycolor(icount), lw=2, ms=10)		# beta!=2
		plt.plot([x[0], x[xstart], x[-1]], [lf1.a +lf1.b*x[0], lf1.a +lf1.b*x[xstart], lf1.a +lf1.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=15)
		plt.figure(fignums[1])
		#plt.title('$\\beta=2.0$')
		#plt.plot(x[1:], y2[1:], '%s.-' % yp.pycolor(icount), label='$\\rho = %s, \\epsilon=%s, D=%s, b=%s$' % (str(thisrho), str(pim), str(beta), str(-lf2.b)[0:5]), lw=2)		# beta=2 approximation
		plt.plot(x[1:], y2[1:], '%s.-' % yp.pycolor(icount), label='$\\epsilon=%s, b=%s$' % (str(pim), str(-lf2.b)[0:5]), lw=2, ms=10)		# beta=2
		plt.plot([x[0], x[xstart], x[-1]], [lf2.a +lf2.b*x[0], lf2.a +lf2.b*x[xstart], lf2.a +lf2.b*x[-1]], '%s|-' % yp.pycolor(icount), ms=15)
		icount+=1
		pim0+=pimrange[2]
		#
		#thisrho+=rhorange[2]
	#
	for fignum in fignums:
		plt.figure(fignum)
		ax=plt.gca()
		plt.subplots_adjust(bottom=.15)
		plt.setp(ax.get_xticklabels(), fontsize=fsize)
		plt.setp(ax.get_yticklabels(), fontsize=fsize)
		
		#lgd=plt.legend(loc='best', numpoints=1, title='$D=1.25$', prop=thisfont2, ncol=1)
		lgd=plt.legend(loc='best', numpoints=1, prop=thisfont2, ncol=1)
		lgd.set_title('$D=1.25$')
		plt.setp(lgd.get_title(), fontsize=fsize)
		if fignum>0: continue
		plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/mfi-fractalType-varEps-bc%s-D125.png' % str(bclust).replace('.', ''))
		plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/mfi-fractalType-varEps-bc%s-D125.eps' % str(bclust).replace('.', ''))
	plt.show()
	
	
	return bofpim

def doFig6(bclust=2.2, kmaxFP=10**9, nmax=200, pimrange=[1.,4.9,.01], betarange=[.9, 1.5, .01]):
	getMFIdistBetaEps(bclust, kmaxFP, nmax, pimrange, betarange)
	
def getMFIdistBetaEps(bclust=2.2, kmaxFP=10**9, nmax=200, pimrange=[0,4.9,.05], betarange=[.9, 2.1, .05]):
	#
	# get contour plot for b(D, epsilon) (Fig 6 in PRE publication)
	#
	# get a full phase-space sampling of the MFI distributions (aka, for a 3d plot epsilon(pim), beta(D), b)
	# let's use plotMFIdistsBeta() and loop over our range of pim.
	# make a list of betas:
	mybetas=[betarange[0]]
	fsize=18
	#
	allmybs=[]	# concat. all bofpim return arrays into this list.
	while mybetas[-1]<betarange[1]: mybetas+=[mybetas[-1]+betarange[2]]
	
	#print "side lens: %f, %f" % (Xside, Yside)
	#return None
	#
	myepsilons=[]
	thispim=pimrange[0]
	while thispim<=pimrange[1]:
		myepsilons+=[thispim]
		bs=plotMFIdistsBeta(bclust, kmaxFP, nmax, thispim, mybetas)
		allmybs+=bs
		thispim+=pimrange[2]
	# allmybs -> [[epsilon, D, a, b, meanvar(b?), a_ln, b_ln, meanvar(b?)_ln] .... ]
	#
	Xside=len(mybetas)
#	#Yside=((pimrange[1]-pimrange[0])/pimrange[2])
	Yside=len(allmybs)/Xside
#	z1=map(operator.itemgetter(3), allmybs)
#	z1=scipy.array(z1)
#	#print "type: %s" % type(z1a).__name__
#	z1.shape=(Yside, Xside)
	#
	X=map(operator.itemgetter(0), allmybs)
	Y=map(operator.itemgetter(1), allmybs)
	Z=map(operator.itemgetter(3), allmybs)
	Z=scipy.array(Z)
	Z.shape=(Yside, Xside)
	#	
	plt.figure(2)
	# get some contours so we can draw labels?
	plt.clf()
	ax=plt.gca()
	plt.subplots_adjust(bottom=.15)
	plt.setp(ax.get_xticklabels(), fontsize=fsize)
	plt.setp(ax.get_yticklabels(), fontsize=fsize)
	
	plt.ylabel("$\\epsilon$", size=fsize+5)
	plt.xlabel("$D$", size=fsize)
	#plt.pcolor(z1, cmap=plt.get_cmap('spectral'))
	#plt.contourf(X,Y,Z, cmap=plt.get_cmap('spectral'))
	C1=plt.contourf(mybetas,myepsilons,Z, cmap=plt.get_cmap('spectral'))
	C2= plt.contour(mybetas,myepsilons,Z)
	clrs=['w', 'w', 'w', 'k', 'k']
	plt.clabel(C2, inline=1, fontsize=fsize, colors=clrs)
	cbar = plt.colorbar(C1)
	plt.setp(cbar.ax.get_yticklabels(), fontsize=fsize)
	cbar.ax.set_xlabel('$b$', size=fsize)
	#plt.colorbar()
	plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/Depsilon-b-contours.png')
	plt.savefig('writeup/mfi-aps/mfi-PRE-letter/figs/Depsilon-b-contours.eps')
	
	return allmybs


def plotTheorybvals():
	#bs=[]
	#bs+=[plotMFIdists(2.2, .56, 10**6, 100, [0.0, 5.0], [True, True, True])]
	#bs+=[plotMFIdists(2.2, .75, 10**6, 100, [0.0, 5.0], [True, True, True])]
	#bs+=[plotMFIdists(2.2, 1.0, 10**6, 100, [0.0, 5.0], [True, True, True])]
	plt.figure(2)
	lns=[]
	lineNum=0
	#rhos=[.1, .2, .3, .4, .5, .6, .7, .8, .9, 1.0]
	rhos=[.1, .3, .6, .8, 1.0]

	#lns+=[plotMFIdists(2.2, .05, 10**6, 100, [0.0, 4.95], [True, True, True], 1, lineNum)]
	#print lns[-1][0], lns[-1][-1]
	#lineNum+=1
	lns+=[plotMFIdists(2.2, .1, 10**6, 200, [0.0, 4.95], [True, True, True], 1, lineNum)]
	lineNum+=1
#	lns+=[plotMFIdists(2.2, .2, 10**6, 200, [0.1, 4.95], [True, True, False], 1, lineNum)]
#	lineNum+=1
	lns+=[plotMFIdists(2.2, .3, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
	lineNum+=1
#	lns+=[plotMFIdists(2.2, .4, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
#	lineNum+=1
#	lns+=[plotMFIdists(2.2, .5, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
#	lineNum+=1
	lns+=[plotMFIdists(2.2, .6, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
	lineNum+=1
#	lns+=[plotMFIdists(2.2, .7, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
#	lineNum+=1
	lns+=[plotMFIdists(2.2, .8, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
	lineNum+=1
#	lns+=[plotMFIdists(2.2, .9, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
#	lineNum+=1	
	lns+=[plotMFIdists(2.2, 1.0, 10**6, 200, [0.0, 4.95], [True, True, False], 1, lineNum)]
	lineNum+=1
	#
	branges=[]
	branges1=[]
	for x in lns:
		branges+=[[x[0][0], x[0][2], x[0][5], x[-1][0], x[-1][2], x[-1][5]]]
		branges1+=[[x[1][0], x[1][2], x[1][5], x[-2][0], x[-2][2], x[-2][5]]]
	#
	plt.legend(loc='upper left')
	#
	plt.figure()
	plt.plot(rhos, map(operator.itemgetter(4), branges), '.-', label='$\\beta = 2.2$', lw=2, ms=10)
	plt.plot(rhos, map(operator.itemgetter(5), branges), '>-', label='$\\beta = 2.0$', lw=2, ms=5)
	plt.ylabel('$b_{max}$', size=18)
	plt.xlabel('$\\rho$', size=18)
	plt.legend(loc='upper right')
	plt.show()	
	#return None
	return [branges, branges1]

def plotMFIdists2(bclust=2.2, rho=.59, kmaxFP=10**6, nMax=100, pimrange=[0.0, 5.0], doClf=[True, True, False], fpmodel=0):
	# MFI dists with variable b(pim)
	# for now, just approximate values of bclust:
	bclust=[[0.0, 2.2], [1.0, 2.22], [2.0, 2.24], [3.0, 2.27], [4.0, 2.35], [4.6, 2.37]]
	# and for now, we know we're going to do .2 intervals, so:
	mybclusts=[]
	for i in xrange(len(bclust)):
	#for rw in bclust:
		rw=bclust[i]
		mybclusts+=[rw]
		if mybclusts[-1]==bclust[-1]: break
		#print mybclusts[-1]
		while mybclusts[-1][0]<bclust[i+1][0]: mybclusts+=[[mybclusts[-1][0]+.2, mybclusts[-1][1]]]
	#return mybclusts
	#
	
	pim=pimrange[0]
	bofpim=[]
	plt.figure(0)
	if doClf[0]: plt.clf()
	plt.figure(1)
	if doClf[1]: plt.clf()

	i=0
	while pim<=pimrange[1]:
		#X=getMFIsquarePropModel(bclust, rho, pim, kmaxFP, nMax)
		X=getMFIsquarePropModel(mybclusts[i][1], rho, pim, kmaxFP, nMax, fpmodel)
		x=[]
		y1=[]
		y2=[]
		for rw in X:
			#print rw
			x+=[math.log(rw[2])]
			y1+=[math.log(rw[3])]
			y2+=[math.log(rw[4])]
		lf1=yp.linefit([x[1:],y1[1:]])
		lf2=yp.linefit([x[1:],y2[1:]])

		lf1.doFit()
		lf2.doFit()
		#
		bofpim+=[[pim, lf1.a, -lf1.b, lf1.meanVar(), lf2.a, -lf2.b, lf2.meanVar()]]
		
		plt.figure(0)
		#plt.loglog(map(operator.itemgetter(2), X), map(operator.itemgetter(3), X), '.-')
		plt.plot(x,y1,'.-', label='pim=%s, b=%s' % (str(pim), str(-lf1.b)))
		plt.figure(1)
		#plt.loglog(map(operator.itemgetter(2), X), map(operator.itemgetter(4), X), '.-')
		plt.plot(x, y2, '.-', label='pim=%s, b=%s' % (str(pim), str(-lf2.b)))
		#
		pim+=0.2
		i+=1
	plt.figure(0)
	plt.legend(loc='upper right')
	plt.figure(1)
	plt.legend(loc='upper right')
		
	plt.figure(2)
	plt.clf()
	plt.plot(map(operator.itemgetter(0), bofpim), map(operator.itemgetter(2), bofpim), '.-', label='$ beta =2.2, rho = %s $' % str(rho), lw=2)
	plt.plot(map(operator.itemgetter(0), bofpim), map(operator.itemgetter(5), bofpim), '.-', label='$ beta =2.0,  rho = %s $' % str(rho), lw=2)
	plt.xlabel('$p_{immune}$', size=16)
	plt.ylabel('$b_{fires}$', size=16)
	plt.legend(loc='best')
		
	plt.show()
	#
	return bofpim
	

	
