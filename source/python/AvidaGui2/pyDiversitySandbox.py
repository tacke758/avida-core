#!/Users/welsberr/avida/avida-ed/Avida-Ed-onestep-qt3/Build/AvidaOneStepLocal/bin/python

# Python program to test various diversity measures

import os 
import sys
import math

import random                                                                                                                                                                                                                      
# From tutorial:
# >>> random.choice(['apple', 'pear', 'banana'])
# 'apple'
# >>> random.sample(xrange(100), 10)   # sampling without replacement
# [30, 83, 16, 4, 8, 81, 41, 50, 18, 33]
# >>> random.random()
# random float
# 0.17970987693706186
# >>> random.randrange(6)    
# random integer chosen from range(6)

from UserDict import UserDict

class UnrealWorld():
    """
    UnrealWorld provides a class for generating an x by y grid of random
    or mutated genotypes and phenotypes.
    """
    # Class properties 
    genealts = "abcdefghijklmnopqrstuvwxyz"
    phenalts = "01"
    slength = 50
    mutrate = 0.02
    uw = []

    # Class methods

    def __init__(self,x=300,y=300,mutrate=0.02):
        """
        __init__ for UnrealWorld, sets up genome and phenotypes for a randomly generated
        population.
        """
        parent = ""
        for ii in range(x):         # (ii = 0; ii < x; ii++):
            for jj in range(y):     # (jj = 0; jj < y; jj++):
                self.uw[ii][jj] = self.mutgenome(self.slength,parent,self.genealts,self.mutrate)
                print self.uw[ii][jj]
                print "\n"

    def mutgenome(self, slength=50, parent="", alts="01", mutrate=0.02):
        """
        mutgenome generates a new genome of a specified length. If the parent
        genome is blank, all bases are randomly determined. Else the parent 
        genome is copied except if a random variable fals below the mutation
        rate, in which case it is randomly picked.
        """
        # Start with a blank string
        mystr = ""
        # Now copy or generate bases as needed
        for kk in range(0, slength-1):      # (kk = 0; kk < slength; kk++):
            myrand = random.random()
            if ((kk >= len(parent)) or (myrand <= mutrate)):
                # Select newbase from alternatives
                mystr += alts[random.randrange(len(alts))]
            else:
                mystr += parent[kk]
        return mystr

class diversity():
    """
    diversity: a class to provide a variety of methods for measures of diversity

    There are a number of arguments given at Lou Jost's site that look convincing
    that there is a distinction between a diversity index and a diversity measure.
    Jost gives the formulae for converting common diversity indices into diversity
    measures, which have the form of estimates of effective number of species. Therefore,
    I am adding those to the diversity results from various methods here.

    Site: http://www.loujost.com/Statistics%20and%20Physics/Diversity%20and%20Similarity/DiversitySimilarityHome.htm
    """
    def __init__(self):
        """
        __init__ there is no initialization required
        """
        pass

    def factorial(self,x):
        """
        factorial
        Provides a factorial function to handle large values of x. Essentially chooses
        to approximate the factorial when x exceeds a threshold.
        """
        # Regular factorial processing
        # if (300 > x):
        if (0):
            xf = 1;
            for ii in range(2,x+1):
                xf *= ii
        else:                     # Too big, so aproximate
            # Stirling's approximation
            # n! ~ n^n e^(-n) sqrt(2 * pi * n) (1 + 1/(12n))
            xf = (x ** x) * (math.e ** (-x)) * math.sqrt(2 * math.pi * x) * (1.0 + 1.0 / (12 * x))
        return xf


    def nmatches(self,s1,s2):
        """
        nmatches

        Returns the number of characters that match between
        two strings.
        """
        mc = 0
        mn = min(len(s1),len(s2))
        for ii in range(mn):
            if (s1[ii] == s2[ii]):
                mc += 1
        return((mc,mn))

    def ndiffs(self,s1,s2):
        """
        ndiffs

        Returns the number of differences between two strings.
        """
        (mc,mn) = self.nmatches(s1,s2)

        return ((mn - mc, mn))

    def addToDictWithIncrement(self,dict,key):
        """
        addToDict

        It is common to use a dict to collect frequency information,
        where each added key matching a previously added key
        increments the associated value.  This is fine except for the
        case of a thus-far-unique key not yet in the dictionary, which
        has to be handled differently in order not to throw the
        KeyError exception. Python dictionaries are touchier than Perl
        hashes, and the initial entry cannot trigger a retrieval of a
        null value; a KeyError exception is thrown.  Thus, all
        additions need to be handled in a way that eliminates the
        KeyError on new key problem.
        """
        a = dict.get(key,0)
        a += 1
        dict[key] = a


    def qlambda(self,n,q,pop):
        """
        qlambda measure at basis of all diversity metrics
        Based on paper by Lou Jost

        Inputs :
        n, number of samples, if n==0 then do the whole population
        q, the order, this is what frequencies are raised to
        pop, the population to work on
        """
        popsize = len(pop)
        if ((n > popsize) or (0 >= n)):
            n = popsize

        # Load xotypes with all the strings we want to analyze, either
        # a sample or the whole population
        # This assumes that n << popsize when n < popsize. There could be
        # trouble with this is n is only slightly smaller than popsize, 
        # as it then is essentially trying to find a rare value by random
        # search. To fix that, an alternate method within the first
        # if clause should handle putting together the indices that would
        # not be used, and then filling xotypes with the complementary
        # set of indices.
        # Initial workaround: limit restricted sample case to less than
        # half the total population. This should keep the efficiency 
        # reasonably high.
        if (n < (popsize * 0.5)):             # Sample point randomly without replacement

            ndx = -1                          # Initial assignment should throw an error if used
            xotypes = {}                      # Clear the temporary dictionary for genotype or phenotype
            ndxs = {}                         # Clear the temporary dictionary of indices

            for ii in range(n):               # Find an unused index for every sample point
                done = 0                      # Initialize control flag for index selection

                while (0 == done):            # Repeat until we have an unused index
                    ndx = random.randrange(n) # Pick a candidate randomly
                    a = ndxs.get(ndx,0)       # Has it already been picked?

                    if (0 == a):              # New index, accept it
                        done = 1              # Flag to stop the while loop

                self.addToDictWithIncrement(xotypes,pop[ndx]) # Add the genotype/phenotype to the dict
                self.addToDictWithIncrement(ndxs,ndx)         # Add the index to the used index dict
        else:
            xotypes = {}                                # Clear the temporary dict
            for ii in pop:                              # Repeat for every entry in the population
                self.addToDictWithIncrement(xotypes,ii) # Add to the dict

        # Now the strings to analyze are in xotypes, so do the analysis
        psum = 0.0
        # Try it with a list completion
        psum = math.sum([(v / n) ** q  for k, v in xotypes.iteritems()])

        return((psum,n))


    def base_diversity(self,pop):
        """
        base_diversity

        This calculates the diversity described as "gene diversity" in email
        on Avida-ED capability.

        For each base, calculate h' on the frequencies of bases seen among all
        members of the population.
        """
        n = len(pop)      # Find the total population size
        #print "len(pop) = %d" % (n)

        # Estimate upper bound on length of genome
        bc = len(pop[0]) * 1       # Factor of 1 assumes all strings are the same length

        hs = []                    # Clear the h' array
        for ii in range(bc):       # For each base
            # print "ii = %d" % (ii) 
            basecnt = {}           # Clear the base counting dict
            basen = 0              # Clear the counter for number of samples
            
            for jj in range(n):    # For every member of the population
                # Only process if the length is right
                if (bc <= len(pop[jj])):
                    #print "pop[%d][%d], pop[%d] = %s" % (jj,ii,jj,pop[jj])
                    #print "jj = %d, len(%s) = %d, char[%d] = %s" % (jj,pop[jj],len(pop[jj]),ii,pop[jj][ii])
                    #print "continuing"
                    self.addToDictWithIncrement(basecnt,pop[jj][ii])  # Increment a counter for each base found
                    basen += 1    # Increment the count of bases 

            # Initialize temporary variables
            psum2 = 0.0
            pf = 0.0
            pf2 = 0.0
            h = 0.0

            for kk in basecnt:            # For every different base observed
                pf = (basecnt[kk] * 1.0) / (basen * 1.0)  # Calculate the frequency of the base
                pf2 = pf * pf             # Calculate the squared frequency
                psum2 += pf2              # Add to a running sum of the squared frequencies
                h = 1.0 - psum2           # Find h' for this base as 1 - sum of squared frequencies
                hs.append(h)                # Save h' in array

        hhat = 0.0           # Initialize the whole-string H estimate
        for ii in hs:        # For all the h's
            hhat += ii       # Add to a sum
        hhat = hhat / bc     # Take the average h' per base
            
        #print "hhat = %7.5f, basen = %d" % (hhat,basen)

        return(hhat)         # Return the estimated h' per base

    def tide_diversity(self,pop,s):
        """
        tide_diversity

        This calculates the diversity measure described as "nucleotide
        diversity" in email on Avida-ED capability.

        This compares pairs of Avidians and calculates h' based on the 
        scaled proportion of differences between the Avidians.

        Inputs:
        pop : array of strings representing genotypes, phenotypes, or whatever.
        s : sample size, 0 = whole population, 0.0 < s < 1.0 is a proportion, otherwise an absolute number of samples
        """

        popsize = len(pop)
        totalsamples = ((popsize * popsize) - popsize) * 0.5    # Triangular non-identical pairs
        pcnt = 0
        dcsum = 0.0

        if (s == 0):
            """
            Do the whole population.
            """
            #print "tide_diversity: whole population"
            for ii in range(len(pop)):
                for jj in range(len(pop)):
                    if (ii < jj):                
                        pcnt += 1
                        # print "Getting diffs for $pop[$ii] and $pop[$jj]\n"
                        (dc,dm) = self.ndiffs(pop[ii],pop[jj])
                        dcsum += dc / len(pop[ii])

        else:

            pairs = {}   # Clear the pairs dict
            if (1.0 <= s):
                """ 
                Sample a specific number of pairs chosen randomly.
                """
                samples = int(min(s,totalsamples,popsize))
                #print "tide_diversity: specific sample of population = %d" % (samples)
            else:
                """
                Sample a fraction of the available pairs.
                """

                tsamp = s * totalsamples
                samples = int(min(max(s * totalsamples,1),popsize))
                #print "totalsamples=%d, proportion=%7.5f, intermediate=%7.5f, samples=%d" % (totalsamples,s,tsamp,samples)
                #print "tide_diversity: proportion of population = %d" % (samples)                
            if (samples < totalsamples):
                #print "samples = " + str(samples) + ",totalsamples = " + str(totalsamples)
                #print "finding %d samples out of " % (samples) + str(totalsamples)
                for ii in range(samples):
                    # Find an unexamined pair
                    done = 0
                    while (0 == done):
                        r1 = random.randrange(popsize)
                        r2 = random.randrange(popsize)
                        
                        if (r1 != r2):
                            a = min(r1,r2)
                            b = max(r1,r2)
                            # Are those actual values in the population?
                            if (0 < min(len(pop[a]),len(pop[b]))):
                                k = "%d;%d" % (a,b)
                                self.addToDictWithIncrement(pairs,k)
                                if (1 == pairs[k]):
                                    done = 1
                        
                    pcnt += 1
                    # print "Getting diffs for $pop[$ii] and $pop[$jj]\n"
                    (dc,dm) = self.ndiffs(pop[a],pop[b])
                    dcsum += (dc * 1.0) / (len(pop[a]) * 1.0)
                    #print "difference count = %d, dcsum = %7.5f" % (dc,dcsum)
        
        if (0 < pcnt):
            ave_pi = dcsum / (pcnt * 1.0)
        else:
            ave_pi = 0.0
        return(pcnt,ave_pi)  # Return the number of pairs and the average

    def shannon_via_q(self,n,q,pop):
        h = 0.0
        res = self.qlambda(n,q,pop)
        return((n,h))



    def shannon_mf(self,aos,lnbase):
        """
        shannon_mf
        This method finds the Shannon diversity measure H based on the 
        machine formula provided by Lloyd et al. 1968.
        
        Input: aos = an array of strings
        All needed properties are extracted from those found in the array of
        strings provided.
        """
        # H' = (c / N) * (log(N!) - sum(log(n_i)))

        # Set c as the conversion factor to change the logarithm base
        c = lnbase
        # Find N as the number of strings in aos
        
        # Collect the frequencies of appearance of each non-empty string
        # Do this by setting up a temporary dictionary
        mydict = {}    # Clear the temporary dict
        
        N = 0          # Initialize N
        for ii in aos: # For every string in the array/list
            if ("" != ii):                             # If the string is non-empty
                # print "adding %s to dict" % (ii)     # Debug output
                self.addToDictWithIncrement(mydict,ii) # Add it to the frequency-tracking dict
                N += 1                                 # Increment the string count
         
        # print "N = %d" % (N)                           # Debug output

        # Now sum up the individual frequency data
        # This could require taking the factorial of a very large number,
        # so use an approximation if the result will be very large
        thesum = 0.0;                 
        #for key in range(**mydict):
        #    value = mydict{key}
        #    thesum += math.log(self.factorial(value))

        # Get the sum via list comprehension
        # thesum = sum(math.log(self.factorial(elem)) for elem in mydict.values)

        
        # print "summing up"      # Debug output
        if (0):                 # So don't do it this way
            for k, v in mydict.iteritems():
                thesum += math.log(self.factorial(v))
            theh = (lnbase / N) * (math.log(self.factorial(N)) - thesum)

        # Old school
        p = 0.0                            # Initialize frequency
        psum = 0.0                         # Initialize sum of frequencies
        for k, v in mydict.iteritems():    # For every string that appears
            # $p = $gens{$_} / $popsize;   
            # $psum += $p * log($p);
            p = float(v) / N               # Calculate the frequency 
            psum += p * math.log(p)        # Add to the summation

        theh = - psum                      # Negate to make it H'

        ens = math.exp(theh)               # Get effective number of species per Jost

        return ((theh,N,ens))

    def ginisimpson(self,aos):
        """
        ginisimpson
        

sub find_div2 {
    local(@pop) = @_;
    local($_,$ii,$jj,$pcnt);

    $pcnt = 0;
    $dcsum = 0.0;
    for ($ii = 0; $ii < $#pop; $ii++) {
	for ($jj = $ii+1; $jj <= $#pop; $jj++) {
	    if ($ii < $jj) {                
		$pcnt++;
		# print "Getting diffs for $pop[$ii] and $pop[$jj]\n";
		($dc,$dm) = &ndiffs($pop[$ii],$pop[$jj]);
		$dcsum += $dc / length($pop[$ii]);
	    }
	}
    }

    $pi = $dcsum / $pcnt;
    print "$pi = $dcsum / $pcnt;\n";

    return($pi,$pcnt);
}
        """
        pcnt = 0 
        dcsum = 0.0
        for ii in range(aos):
            for jj in range(aos):
                if (jj < ii):
                    pcnt += 1
                    # Count differences between two strings
                    (dc, dm) = self.ndiffs(aos[ii],aos[jj])
                    
                    # Normalize for string length and sum
                    dcsum = dc / min(len(aos[ii]),len(aos[jj]))

        pi = dcsum / pcnt
        
        # multiple values being returned, so use a tuple
        return((pi,pcnt))


    def ginisimpson_timed(self,aos):
        """
        ginisimpson_timed

sub find_div2_timed_dt {
    local($seconds,@pop) = @_;
    local($_,$ii,$jj,$pcnt);

    #$t0 = DateTime->now;

    $popsize = $#pop + 1;
    $pcnt = 0;
    $dcsum = 0.0;
    undef(%pairs);
    $td = 0.0;
    do {
	# Get the population indices for a pair randomly
	$trycnt = 0;
	
	$bad = 0;
	do {
	    $ok = 0;
	    $r1 = int(rand($popsize));
	    $r2 = int(rand($popsize));
	    unless ($r1 == $r2) { 
		$ii = &min($r1,$r2);
		$jj = &max($r1,$r2);
		$key = join(";",$ii,$jj);
		unless (defined $pairs{$key}) {
		    $ok = 1;
		}
	    }
	    if ((0.4 * $popsize) < $trycnt++) {
		$bad = 1;
	    }
	} until ($ok || $bad);
	
	# print "$key\n";
	if ($bad) {
	    $pi = $dcsum / $pcnt;
	    # print "$td: $pi = $dcsum / $pcnt;\n";
	    
	    return($pi,$pcnt);
	}

	# Compare them
	$pcnt++;
	($dc,$dm) = &ndiffs($pop[$ii],$pop[$jj]);
	$dcsum += $dc / length($pop[$ii]);
	
	# Are we out of time?
	#$t1 = DateTime->now;
	#$td = $t1 - $t0;
	#$tds = $td->offset;
	$tds = $pcnt / 16000;
	#print "td = $tds, seconds = $seconds\n";
    } while ($seconds > $tds);

    #print "Out of while loop\n";

    $pi = $dcsum / $pcnt;
    print "$tds: $pi = $dcsum / $pcnt;\n";

    return($pi,$pcnt);
}
        """
        

        pass



    
