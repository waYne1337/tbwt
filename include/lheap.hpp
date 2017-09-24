/*
 * lheap.hpp for bwt tunneling
 * Copyright (c) 2017 Uwe Baier All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __LHEAP_HPP
#define __LHEAP_HPP

#include <assert.h>
#include <functional>
#include <utility>

/**
   The lower methods implement a lazy max heap.
   A lazy max heap is a heap that allows, additional to the operations on
   a normal heap, the decrease and removal of heaped elements. Unlike normal
   heaps, decreasing and removing elements will not take place immediately -
   instead, the user provides a function which indicates the state of an element.
   The respective element then will be heapified / removed as soon as it is required
   for a comparison to another element.
  
   The methods require following callback functions:
   - vstate: takes an element (maybe per reference) and returns its state:
             - lheap_vstate::unchanged
               indicates that value was not changed since last usage
             - lheap_vstate::decreased
               indicates that value was decreased and has to be heapified.
               Note that after the call, an additional call to the same value
               has to return lheap_vstate::unchanged.
             - lheap_vstate::empty
               value is no longer present and has to be removed from the heap.
             the default is a function that always returns lheap_vstate::unchanged.
   - comp: takes two elements from heap and returns true if the first element
           is smaller than the second. It is ensured that the state of each
           compared value is lheap_vstate::unchanged. Default for comp is std::less.
  
   Following methods are available:
   - make_lheap: construct the heap from a range of elements
                 (during construction, no values can be decreased)
   - pop_lheap_nomove: removes the first element and rebuilds the heap.
                       during this call, the state of variables will be checked
                       if necessary. Function returns the new ending position of
                       the new heap.
   - pop_lheap: removes the first element and rebuilds the heap.
                Additionally, the first element will be moved to the last
                position. Function returns the new ending position of
                the new heap.
                NOTE ON USAGE: be careful using this function when
                               elements of the heap are removed, as
                               the removed elements behind the heap not
                               necessarily need to be consecutive.
  
   Typical Usage:
   The typical usage of this methods is in situations where the removal of
   the maximal element in a heap causes side-effects on other elements which
   cause them to decrease their weight or even be removed completely.
  
   Example Usage: (requires vector from STL)

vector<int> A{ 10,20,30,5,15,1,1,7,1,3 };
make_lheap( A.begin(), A.end() );
auto vstate = []( int &i ) { 
	if (i > 0)	return lheap_vstate::unchanged;
	else if (i == 0) return lheap_vstate::empty;
	else {
		i = -i; //ensure value is correct after call
		return lheap_vstate::decreased;
	}
};
cout << "size of A: " << A.size() << endl << "A = {";
for (auto a : A) cout << "\t" << a;
cout << "\t}" << endl;

for (auto e = A.end(); e != A.begin(); e = pop_lheap_nomove(A.begin(), e, vstate)) {
	//output current maximum
	cout << A.front() << endl;
	//decrease a random variable from mid
	int i = (e - A.begin()) / 2;
	cout << "Decreasing value " << A[i] << " to half" << endl;
	A[i] = -(A[i] / 2);		
}

 */

//! Value State
class lheap_vstate {
	public:
		//!unchanged state, value is still the same
		const static int unchanged = 0;
		//!decreased state, value needs to be heapified 
		const static int decreased = 1;
		//!empty state, value needs to be removed
		const static int empty     = 2;
	private:
		//define functions as friends
		template<class RAI,class C>
		friend void make_lheap(RAI,RAI,C);
		template<class RAI,class VS, class C> 
		friend RAI push_lheap(RAI,RAI,VS,C);
		template<class RAI,class VS,class C>
		friend RAI pop_lheap_nomove(RAI,RAI,VS,C);

		//and offer a couple of helper methods

		//! heapify element at position i
		template<class RAI, class VS, class C>
		static void heapify( RAI b, typename RAI::difference_type i, 
		                     typename RAI::difference_type &n,
		                     VS vstate, C comp );

		//! replace empty element at position i with last element
		/*! returns true if replacement worked, false if
		   no non-empty elements were found for replacement
		*/
		template<class RAI, class VS, class C>
		static bool replace( RAI b, typename RAI::difference_type i, 
		                     typename RAI::difference_type &n,
		                     VS vstate, C comp );

		//! ensure element at position i has correct value (an unchanged state)
		/*! returns true if value is correct, false if heap contains only
		   empty elements
		*/
		template<class RAI, class VS, class C>
		static inline bool ensureValue( RAI b, typename RAI::difference_type i, 
		                                typename RAI::difference_type &n,
		                                VS vstate, C comp );
};

//// MAKE HEAP ////////////////////////////////////////////////////////////////

//! constructs a lazy heap. No value of any element can be decreased during construction.
template <class RandomAccessIterator, class Compare>
void make_lheap (RandomAccessIterator first, RandomAccessIterator last,
                 Compare comp) {
	typedef typename RandomAccessIterator::value_type v_type;
	typedef typename RandomAccessIterator::difference_type d_type;

	//values remain unchanged on construction
	auto vstate = []( v_type v ) { (void)(v); return lheap_vstate::unchanged; };
	d_type n = last - first;

	//build the heap in a standard manner
	for (d_type i = n / 2; i >= 0; --i)
		lheap_vstate::heapify( first, i, n, vstate, comp );
}

//! constructs a lazy heap. No value of any element can be decreased during construction.
template <class RandomAccessIterator>
void make_lheap (RandomAccessIterator first, RandomAccessIterator last) {
	make_lheap(first,last,std::less<typename RandomAccessIterator::value_type>());
}

//// POP FROM HEAP WITH NO MOVE ///////////////////////////////////////////////

//! removes greatest element from heap but does not move it to last.
/*! This is especially useful if elements are removed from heap, as last
   will not sequentially be decremented.
   returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator, class ValueState, class Compare>
RandomAccessIterator pop_lheap_nomove(RandomAccessIterator first,
                               RandomAccessIterator last,
                               ValueState vstate, Compare comp) {
	assert(first != last);
	typedef typename RandomAccessIterator::difference_type d_type;
	d_type n = last - first;
	if (n == 0) assert(false);
	lheap_vstate::replace( first, (d_type)0, n, vstate, comp );
	return first + n;
}

//! removes greatest element from heap but does not move it to last.
/*! This is especially useful if elements are removed from heap, as last
   will not sequentially be decremented.
   returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator, class ValueState>
RandomAccessIterator pop_lheap_nomove(RandomAccessIterator first,
                               RandomAccessIterator last,
                               ValueState vstate) {
	return pop_lheap_nomove(first,last,vstate,
		std::less<typename RandomAccessIterator::value_type>());
}

//! removes greatest element from heap but does not move it to last.
/*! This is especially useful if elements are removed from heap, as last
   will not sequentially be decremented.
   returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator>
RandomAccessIterator pop_lheap_nomove(RandomAccessIterator first,
                               RandomAccessIterator last) {
	typedef typename RandomAccessIterator::value_type value_type;
	return pop_lheap_nomove(first,last,[]( value_type v )
		{ (void)(v);return lheap_vstate::unchanged; });
}

//// POP FROM HEAP ////////////////////////////////////////////////////////////

//! removes greatest element from heap and moves it before last.
/*! returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator, class ValueState, class Compare>
RandomAccessIterator pop_lheap(RandomAccessIterator first,
                               RandomAccessIterator last,
                               ValueState vstate, Compare comp) {
	assert(first != last);	
	auto v = *first;
	auto r = pop_lheap_nomove(first, last, vstate, comp);
	*(--last) = v;
	return r;	
}

//! removes greatest element from heap and moves it to last.
/*! returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator, class ValueState>
RandomAccessIterator pop_lheap(RandomAccessIterator first,
                               RandomAccessIterator last,
                               ValueState vstate) {
	return pop_lheap(first,last,vstate,
		std::less<typename RandomAccessIterator::value_type>());
}

//! removes greatest element from heap and moves it to last.
/*! returns the new position of last in the remaining heap, 
   as potentially a couple of elements are removed from the heap.
*/
template<class RandomAccessIterator>
RandomAccessIterator pop_lheap(RandomAccessIterator first,
                               RandomAccessIterator last) {
	typedef typename RandomAccessIterator::value_type value_type;
	return pop_lheap(first,last,[]( value_type v ) 
		{ (void)(v);return lheap_vstate::unchanged; });
}

//// IMPLEMENTATION OF HEAP LOGIC /////////////////////////////////////////////

template<class RAI, class VS, class C>
void lheap_vstate::heapify( RAI b, typename RAI::difference_type i, 
		            typename RAI::difference_type &n,
		            VS vstate, C comp ) {

	typename RAI::difference_type k;
	do {
		k = 2 * i + 1;

		//ensure value k is existing
		if (k >= n || !ensureValue( b, k, n, vstate, comp ))
			return; //first value not present, so stop already

		//ensure value k+1 is existing
		if (k+1 >= n || !ensureValue( b, k+1, n, vstate, comp )) {
			//value not present, so just check first value against i
			if (comp(b[i], b[k])) { //i is smaller than k, so swap values
				std::swap( b[i], b[k] );
			}
			return;
		}

		//both values k and k+1 present, so get bigger one
		if (comp(b[k], b[k+1])) ++k; //k is smaller than k+1

		//compare i with maximum of k and k+1
		if (comp(b[i], b[k])) { //i is smaller than k, swap and proceed on
			std::swap( b[i], b[k] );
			i = k;
		}
	} while (i == k);
}

template<class RAI, class VS, class C>
bool lheap_vstate::replace( RAI b, typename RAI::difference_type i, 
		            typename RAI::difference_type &n,
		            VS vstate, C comp ) {
	do {
		if (vstate(b[--n]) != empty) { //find a non-empty value at back
			b[i] = b[n]; //copy value to front
			heapify( b, i, n, vstate, comp ); //and put it to the correct place
			return true;
		}
	} while (i < n);
	return false;
}

template<class RAI, class VS, class C>
inline bool lheap_vstate::ensureValue( RAI b, typename RAI::difference_type i, 
                                       typename RAI::difference_type &n,
                                       VS vstate, C comp ) {
	//check state and decide what to do
	switch (vstate(b[i])) {
	case unchanged: //state fine
		return true;
	case decreased: //heapify value, as it was decreased
		heapify( b, i, n, vstate, comp );
		return true;
	case empty: //value no longer existing, replace it
		return replace( b, i, n, vstate, comp );
	}
	assert(false); //stop execution, as this is undefined behaviour
	// (vstate has to return one of the three integers up there)
	return false;
}

#endif
