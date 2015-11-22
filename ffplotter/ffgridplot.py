import matplotlib.pyplot as plt
import operator
import math
import glob
import random
import os

import scipy
import numpy

class ffgridplot():
	#grid=None
	#gridPolys=[]
	plt.ion()
	
	def __init__(self, X, Y, inDataDir='data', fignum=0, tcolor='g', fcolor='r', ecolor='k'):
		self.fg=plt.figure(fignum)
		plt.clf()
		self.axs=[]
		self.axs+=[self.fg.add_axes([.1,.1, .9, .25])]
		self.axs+=[self.fg.add_axes([.1, .3, .9, .65])]
		
		self.griddata=[]
		self.gridPolys=[]
		self.rhots=[]
		self.X=X
		self.Y=Y
		
		self.fcolor=fcolor
		self.tcolor=tcolor
		self.ecolor=ecolor
		self.fignum=0
		
		self.inDataDir=inDataDir
		self.datafiles=glob.glob(inDataDir)
		
		self.initGrid(X, Y, fignum, self.axs[1])
		
	def initGrid(self, X=None, Y=None, fignum=None, ax=None):
		if X==None: X=self.X
		if Y==None: Y=self.Y
		if fignum==None: fignum=self.fignum
		if ax==None: ax=self.axs[1]
		self.gridPolys=[]
		#
		for i in xrange(X*Y):
			x=i%X
			y=int(i/Y)
			verts=self.getSquare(x,y,1)
			self.gridPolys+=ax.fill(verts[0], verts[1], 'k', alpha=.4)
			self.griddata+=[[i, 0]]
			if i%100==0: print "setting patches, %d" % i
		
	def getSquare(self, x,y,l=1):
		# consider x,y LL corner.
		#return [[x,y], [x+l, y], [x+l, y+l], [x, y+l], [x,y]]
		return [[x, x+l, x+l, x, x], [y, y, y+l, y+l, y]]
	
	def setGridvals(self, fin='data/ffdata0.dat'):
		f=open(fin)
		iprev=0
		Ntrees=0
		for rw in f:
			if rw[0]=='#': continue
			#print rw
			rws=rw.split('\t')
			i=int(rws[0])
			z=int(rws[1])
			#
			ii=i
			while ii>(iprev+1) and ii>=0:
				#print "correcting a gap..."
				self.griddata[ii][1]=0
				self.gridPolys[ii].set_color(self.ecolor)
				ii-=1
			#
			clr=self.ecolor
			if z==1:
				clr=self.tcolor
				Ntrees+=1
			if z==-1: clr=self.fcolor
			#if rw[1]==0: clr=self.ecolor
			#
			self.gridPolys[i].set_color(clr)
			iprev=i
		#
		f.close()
		self.rhots+=[[len(self.rhots), float(Ntrees)/float(self.X*self.Y)]]
		self.fg.canvas.draw()
	
	def doFFmovie(self, inDir=None):
		if inDir==None:
			#inDir=self.inDataDir
			fls=self.datafiles
		if inDir!=None:
			fls=glob.glob(inDir)
		#
		for fl in fls:
			self.setGridvals(fl)
	def ffHazmap1(self, datafile='indata/randomgrid.dat', fout='outdata/gridHazmap.dat', eps=1., XY=None):
		# using DSFFM-MFI model, make a fire hazard map of a grid.
		# input file should have some sort of header X=x0, Y=yo.
		# for each square, start a "fire" propagate and calc the expected size of fire,
		# aka: sum(P_i*dk), add up the number of new elements * the prob that they burn.
		#
		# first, read the grid into an array.
		# add a border of unoccupied (or specially occupied) sites - an easy way to avoid stepping off the grid.
		#
		borderval=42
		thisgrid=[]
		fin=open(datafile)
		for ln in fin:
			if 'X=' in ln:
				# header row.
				rws=ln.split(',')	# we'll have to come up with a proper standard eventually.
				X=int(rws[0].split('=')[1])
				Y=int(rws[1].split('=')[1])
				#thisgrid+=[(scipy.zeros(X+1)+borderval).tolist()]	# note: this will be (x+2) elements once we start reading the data file.
				for i in xrange(X+1): thisgrid+=[borderval]
				# l1+=[(fpp.scipy.zeros(10)+43).tolist()]
			if ln[0]=='#': continue
			#
			lns=ln.split()	# we get an index and a value
			indx=float(lns[0])
			z=int(lns[1])
			#
			if indx%X==0:	# end of row
				thisgrid+=[borderval, borderval]
			#
			thisgrid+=[z]
		#thisgrid+=[(scipy.zeros(X+2)+borderval).tolist()]
		for i in xrange(X+3): thisgrid+=[borderval]
		fin.close()
		#
		# two haz-maps: 1) expected fire size, 2) risk to each cell (number of times each cell burns)
		firesizes=scipy.zeros((X+2)*(Y+2)).tolist()
		nburns=scipy.zeros((X+2)*(Y+2)).tolist()	# use arrays or lists?
		#
		# now, start a fire in each square:
		i=X+2
		while i<(len(thisgrid)-(X+2)):
			if thisgrid[i]==1:
				firelist=dofire(thisgrid, i, eps, X)
				thisProbs=map(operator.itemgetter(0), firelist)
				#firecells=map(operator.itemgetter(1), firelist)
				firesizes[i]+=sum(thisProbs)
				#print "new cell."
				#for fc in firecells:
				for rw in firelist:
					nburns[rw[1]]+=rw[0]
				del firelist
			#
			i+=1
		#
		return [thisgrid, firesizes, nburns]

	
def makeRandomGridFile2(X=128, Y=128, rho=.59, foutname='ffindata/randomgrid.dat'):
	imax=X*Y
	r1=random.Random()
	f=open(foutname, 'w')
	f.write("#random grid\n")
	f.write("#X=%d, Y=%d, rho=%f\n" % (X, Y, rho))
	for i in xrange(imax):
		#f.write('%d\t%d\n' % (i, round(r1.random())))
		z=0.
		if r1.random()<=rho: z=1.
		f.write('%d\t%d\n' % (i, z))
	f.close()
	
def makeRandomGridFile(X=128, Y=128, rho=.59, foutname='ffindata/randomgrid.dat'):
	imax=X*Y
	r1=random.Random()
	f=open(foutname, 'w')
	f.write("#random grid\n")
	f.write("#X=%d, Y=%d, rho=%f\n" % (X, Y, rho))
	for i in xrange(imax):
		f.write('%d\t%d\n' % (i, rho*round(3*r1.random()-1.5)))
	f.close()

def ffHazmap1(datafile='indata/randomgrid.dat', fout='outdata/gridHazmap.dat', eps=1., XY=None):
	# using DSFFM-MFI model, make a fire hazard map of a grid.
	# input file should have some sort of header X=x0, Y=yo.
	# for each square, start a "fire" propagate and calc the expected size of fire,
	# aka: sum(P_i*dk), add up the number of new elements * the prob that they burn.
	#
	# first, read the grid into an array.
	# add a border of unoccupied (or specially occupied) sites - an easy way to avoid stepping off the grid.
	#
	borderval=42
	thisgrid=[]
	fin=open(datafile)
	for ln in fin:
		if 'X=' in ln:
			# header row.
			rws=ln.split(',')	# we'll have to come up with a proper standard eventually.
			X=int(rws[0].split('=')[1])
			Y=int(rws[1].split('=')[1])
			#thisgrid+=[(scipy.zeros(X+1)+borderval).tolist()]	# note: this will be (x+2) elements once we start reading the data file.
			for i in xrange(X+1): thisgrid+=[borderval]
			# l1+=[(fpp.scipy.zeros(10)+43).tolist()]
		if ln[0]=='#': continue
		#
		lns=ln.split()	# we get an index and a value
		indx=float(lns[0])
		z=int(lns[1])
		#
		if indx%X==0:	# end of row
			thisgrid+=[borderval, borderval]
		#
		thisgrid+=[z]
	#thisgrid+=[(scipy.zeros(X+2)+borderval).tolist()]
	for i in xrange(X+3): thisgrid+=[borderval]
	fin.close()
	#
	# two haz-maps: 1) expected fire size, 2) risk to each cell (number of times each cell burns)
	firesizes=scipy.zeros((X+2)*(Y+2)).tolist()
	nburns=scipy.zeros((X+2)*(Y+2)).tolist()	# use arrays or lists?
	#
	# now, start a fire in each square:
	i=X+2
	while i<(len(thisgrid)-(X+2)):
		if thisgrid[i]==1:
			firelist=dofire(thisgrid, i, eps, X)
			thisProbs=map(operator.itemgetter(0), firelist)
			#firecells=map(operator.itemgetter(1), firelist)
			firesizes[i]+=sum(thisProbs)
			#print "new cell."
			#for fc in firecells:
			for rw in firelist:
				nburns[rw[1]]+=rw[0]
			del firelist
		#
		i+=1
	#
	return [thisgrid, firesizes, nburns]

def writeData(datain, foutname):
	# for now, keep it simple and just use index values; don't make x,y.
	fout=open(foutname, 'w')
	fout.write('# forest-fire hazard map data\n#i\tz_ffm\tz_h1\tz_h2\n')
	for i in xrange(len(datain[0])):
		fout.write('%d\t%d\t%f\t%f\n' % (i, datain[0][i], datain[1][i], datain[2][i]))
	fout.close()

def getBoxyContour(datafile='indata/randomgrid.dat', eps=1., fignum=0, xrnges=None, XY=None, outdir='outdata1'):
	xrnge1=xrnges[0]
	xrnge2=xrnges[1]
	
	A=ffHazmap1(datafile, None, eps, XY)
	L=int(len(A[0])**.5)
	writeData(A, '%s/hazstats-%d-L%d.dat' % (outdir, int(100*eps), L))
	plt.figure(fignum)
	x1=boxycontour(A[1], None, xrnge1)
	plt.figure(fignum+1)
	x2=boxycontour(A[2], None, xrnge2)
	#x2=boxycontour(A[2])
	
	return A
def boxycontour(grids, X0=None, xrnge=None):
	z=grids[:]
	if X0==None: X0=(len(grids))**.5
	if type(z).__name__!='ndarray': z=scipy.array(z)
	z.shape=(X0, math.ceil(len(z)/X0))
	#
	if xrnge!=None and (type(xrnge).__name__ in ('list', 'tuple', 'ndarray')):
		# now, this part is silly. usually, we have a border row; we might be stomping on a couple of bits
		# BUT, we can control our color spectrum.
		if xrnge[0]!=None: z[0][0]=xrnge[0]	# max val
		if len(xrnge)>1 and xrnge[1]!=None: z[0][1]=xrnge[1]	# min val (this is a silly order, but often we only need to specify the max value)
	#
	plt.pcolor(z, cmap=plt.get_cmap('spectral'))
	plt.colorbar()
	
	

def getContour(datafile='indata/randomgrid.dat', fout='outdata1/gridHazmap.dat', eps=1., XY=None):
	A=ffHazmap1(datafile, fout, eps, XY)
	writeData(A, 'outdata1/hazstats-%d.dat' % int(100*eps))
	#plt.figure(0)
	#x0=ffcontour(A[0])
	plt.figure(0)
	#plt.clf()
	x1=ffcontour(A[1])
	#plt.figure(2)
	#x2=ffcontour(A[2])
	#
	return A
	

def ffcontour(grids, X0=None):
	# if X0==None: assume square
	if X0==None: X0=(len(grids))**.5
	#
	# in this case, X0 is explicitly the width of a row, including borders.
	Xs=[]
	Ys=[]
	Z=scipy.array(grids)
	Z.shape=(X0, math.ceil(len(grids)/X0))
	
	Xs=range(X0)
	Ys=range(X0)
	
	#for i in xrange(len(grids)):
	#	Xs+=[i%X0]
	#	Ys+=[int(i/X0)]
	#	# Z+=[[i%X0, int(i/X0), grids[i]]]
	X,Y=numpy.meshgrid(Xs, Ys)
	#Z.shape=(X0, math.ceil(len(grids)/X0))
	#print "lens: %d * %d = %d, %d" % (len(Xs), len(Ys), len(Xs)*len(Ys), len(grids))
	Z.shape=(len(Xs), len(Ys))
	plt.contourf(X,Y,Z, alpha=.35)
	#plt.colorbar(ticks=[-5,-4,-3,-2,-1])
	plt.colorbar()
	plt.spectral()

def dofire(thisgrid, i, eps, X):
	if thisgrid[i]!=1: return [[0,i]]
	flist=[[1, i]]	# [[probs], [i's]]
	startIndex=0
	endIndex=1		# start/end+1 index for fires
	#dk=1
	P=1.0	# cum prob.
	#print "start a fire."

	while endIndex>startIndex:
	#while dk>0:
		#dk=0
		#endIndex=len(flist)
	#	print "new prim. %d, %d" % (activeIndex[0], activeIndex[1])
		#print " index: %d, %d; flist[]: %s %d" % (activeIndex[0], activeIndex[1], str(flist[activeIndex[0]:activeIndex[1]]), len(flist))
		for elem in flist[startIndex:endIndex]:
		#for i in xrange(activeIndex[0], len(flist)):
			#elem=flist[i]
			#print "elem: %s, %d" % (str(elem), i)
			#if len(flist)>1: print "%d, %d, %f, %d, %d" % (i, len(flist), P, startIndex, endIndex)
			#break
			ii=elem[1]
			# check neighbors:
			indxs=[ii+1, ii-1, ii+X+2, ii-X-2]
			for indx in indxs:
				if thisgrid[indx]==1: 
					flist+=[[P, indx]]
					thisgrid[indx]=-1
					#dk+=1
			#
		#
		# perimeter complete.
		P*=(1.0-(eps/(5.0*float(len(flist)))))
		#activeIndex[0]=activeIndex[1]
		#activeIndex[1]=len(flist)
		startIndex=endIndex
		endIndex=len(flist)
		#print " index again: %d, %d; flist[]: %s" % (activeIndex[0], activeIndex[1], str(flist[activeIndex[0]:activeIndex[1]]))
	#print "fire finished."
	# now, return all fires to trees:
	for rw in flist:
		thisgrid[rw[1]]=1
	#del activeIndex
	#
	return flist

def homelyHazmap(datain='indata1/ffdata-L128-50.dat', dataout='outdata128'):
	# there is a better, prettier, way to do this.
	objf1=ffgridplot(128, 128, 'indata1', 0)
	plt.figure(0)
	eps=0
	while eps<5.0:
		plt.clf()
		plt.title='$\\epsilon = %f$' % eps
		objf1.initGrid()
		objf1.setGridvals(datain)
		getContour(datain, None, eps)
		#plt.title='$\\epsilon = %f$' % eps
		svname='%s/movie-meank/%s-l128.png' % ('000'+str(int(eps*100)))[-3:]
		plt.savefig(svname)
		eps+=.1
		#
		svname2='%s/movie-nburned/%s-l128.jpg' % ('000'+str(int(eps*100)))[-3:]
		os.system('convert %s %s' % (svname, svname2))
	#
	#os.system('mencoder mf://outdata/movie1/*.jpg -mf w=800:h=600:fps=3:type=jpg -ovc copy -o pimovie.avi')
	return objf1

#def prettyHazmap(datain='indata1/ffdata-L128-50.dat', outfolder='outdata1024'):
def prettyHazmap(datain=None, outfolder=None):
	eps=0
	fignum=0
	xrng1=None	# xrange: [maxX, minX]
	xrng2=None
	L=None
	if datain==None: datain='indata1/ffdata-L128-50.dat'
	if outfolder==None: outfolder='outdata1024'
	
	while eps<5.0:
		plt.figure(1)
		plt.clf()
		plt.figure(0)
		plt.clf()
		plt.title='$\\epsilon = %f$' % eps
		#objf1.initGrid()
		#objf1.setGridvals(datain)
		#getContour(datain, None, eps)
		#
		#getBoxyContour(datafile='indata/randomgrid.dat', eps=1., fignum=0, xrnge=None, XY=None):
		A=getBoxyContour(datain, eps, fignum, [xrng1,xrng2], None, outfolder)
		if L==None: L=len(A[0])**.5
		#
		# and get the range:
		# get max,min vals from the first iteration (eps=0), which will be the most extreme values for the sim.
		if xrng1==None: xrng1=[max(A[1]), min(A[1])]
		if xrng2==None: xrng2=[max(A[2]), min(A[2])]
			
		#plt.title='$\\epsilon = %f$' % eps
		plt.figure(fignum)
		svname='%s/movie-meank/%s-l%d.png' % (outfolder, ('000'+str(int(eps*100)))[-3:], L)
		plt.savefig(svname)
		svname2='%s/movie-meank/%s-l%d.jpg' % (outfolder, ('000'+str(int(eps*100)))[-3:], L)
		os.system('convert %s %s' % (svname, svname2))
		#
		plt.figure(fignum+1)
		#plt.title='$\\epsilon = %f$' % eps
		svname='%s/movie-nburned/%s-l%d.png' % (outfolder, ('000'+str(int(eps*100)))[-3:], L)
		plt.savefig(svname)
		svname2='%s/movie-nburned/%s-l%d.jpg' % (outfolder, ('000'+str(int(eps*100)))[-3:], L)
		os.system('convert %s %s' % (svname, svname2))
		#
		eps+=.1
		#
	#
	os.system('mencoder mf://%s/movie-meank/*.jpg -mf w=800:h=600:fps=3:type=jpg -ovc copy -o boxyffhazMeank.avi' % outfolder)
	os.system('mencoder mf://%s/movie-nburned/*.jpg -mf w=800:h=600:fps=3:type=jpg -ovc copy -o boxyffhaznburn.avi' % outfolder)
	
def hazmapContours(fname):
	# plot haz-maps from data files.
	# something like this (as per james):

	indx, grid, haz1, haz2 = [],[],[],[]

	datafile = open(fname)
	nelements=0
	for line in datafile.readlines():
		 if line[0]=='#': continue
		 nelements+=1
		 data = line.split()
		 indx.append( int(data[0]) )
		 grid.append( int(data[1]) )
		 if len(data)>2: haz1.append( float(data[2]) )
		 if len(data)>3: haz2.append( float(data[3]) )
	# for now, assume square lattice:
	L=nelements**.5

	indx = numpy.array(indx)
	#indx.shape = 130,130
	indx.shape=L,L

	grid = numpy.array(grid)
	grid.shape = L,L
	plt.pcolor(grid, cmap=plt.get_cmap('spectral'))

	if len(haz1)>0:
		haz1 = numpy.array(haz1)
		haz1.shape = L,L
		plt.figure(1)
		plt.pcolor(haz1, cmap=plt.get_cmap('spectral'))
		plt.colorbar()

	if len(haz2)>0:
		haz2 = numpy.array(haz2)
		haz2.shape = L,L
		plt.figure(2)
		plt.pcolor(haz2, cmap=plt.get_cmap('spectral'))
		plt.colorbar()

def fmplot(dfile='outdata1/hazstats-250.dat'):
	# get a freq-mag plot (rank order cum dist?) from a data file.
	# the easiest thing to do is sort and count for a CDF, but let's
	# just spin through it twice (or use a max/min function) and actually get the integer counts.
	# if we do this the second way, we don't have to worry about border vals.
	maxz1=0
	maxz2=0	# use these to establish length of bin arrays.
	
	Z1,Z2=[], []
	fin=open(dfile)
	for rw in fin:
		if rw[0]=='#': continue
		rws=rw.split()
		Z1+=[float(rws[2])]
		Z2+=[float(rws[3])]
		#if Z1[-1]>maxz1: maxz1=Z1[-1]
		#if Z2[-1]>maxz2: maxz1=Z2[-1]
	fin.close()
	#
	maxz1=max(Z1)
	maxz2=max(Z2)
	#
	hazZ1=scipy.zeros(maxz1+1)
	hazZ2=scipy.zeros(maxz2+1)
	#
	for z in Z1:
		hazZ1[z]+=1
	for z in Z2:
		hazZ2[z]+=1
	#
	X1=range(len(hazZ1))
	X2=range(len(hazZ2))
	#
	plt.figure(0)
	plt.clf()
	plt.title("haz1")
	plt.loglog(X1, hazZ1)
	plt.figure(1)
	plt.clf()
	plt.title("haz2")
	plt.loglog(X2, hazZ2)
	plt.show()
	#return [hazZ1, hazZ2]
	#
	
		
	
	



