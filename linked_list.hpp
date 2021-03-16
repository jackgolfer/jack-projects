template <typename T>
List < T >::const_iterator::const_iterator()
{
	current = nullptr;
}
template <typename T>
const T & List < T >::const_iterator:: operator*() const
{
	return retrieve();
}
template <typename T>
typename List<T>::const_iterator & List < T >::const_iterator:: operator++()
{
	current = current->next;
	return *this;
}
template <typename T>
typename List<T>::const_iterator List < T >::const_iterator:: operator++(int)
{
	const_iterator old = *this;
	++( *this );
	return old;
}
template <typename T>
typename List<T>::const_iterator & List < T >::const_iterator:: operator--()
{
	current = current->prev;
	return *this;
}
template <typename T>
typename List<T>::const_iterator List < T >::const_iterator::operator--(int)
{
	const_iterator old = *this;
	--( *this );
	return old;
}
template <typename T>
bool List < T >::const_iterator:: operator==(const const_iterator &rhs) const
{
	return current == rhs.current;
}
template <typename T>
bool List < T >::const_iterator:: operator!=(const const_iterator &rhs) const
{
	return !(*this == rhs);
}
template <typename T>
T & List < T >::const_iterator:: retrieve() const
{
	return current->data;
}
template <typename T>
List < T >::const_iterator:: const_iterator(Node *p)
{
	current = p;
}
//------------------------------------------------------------------------------------
template <typename T>
List < T >:: iterator::iterator()
{
}
template <typename T>
T & List < T >:: iterator:: operator*()
{
	return const_iterator::retrieve();
}
template <typename T>
const T & List < T >:: iterator:: operator*() const
{
	return const_iterator::operator*( );
}
template <typename T>
typename List<T>::iterator & List < T >:: iterator:: operator++()
{
	this->current = this->current->next;
	return *this;
}
template <typename T>
typename List<T>::iterator List < T >:: iterator:: operator++(int)
{
	iterator old = *this;
	++( *this );
	return old;
}
template <typename T>
typename List<T>::iterator & List < T >:: iterator:: operator--()
{
	this->current = this->current->prev;
	return *this;
}
template <typename T>
typename List<T>::iterator List < T >:: iterator:: operator--(int)
{
	iterator old = *this;
	--( *this );
	return old;
}
template <typename T>
List < T >:: iterator::iterator(Node *p)
{
	const_iterator::current = p;
}
//-------------------------------------------------------------------------------
template <typename T>
List < T >::List()
{
	init();
}
template <typename T>
List < T >::List(const List & rhs)
{
	init();
	for (auto & x : rhs)
		push_back(x);
}
template <typename T>
List < T >::List(List && rhs)
{
	theSize = rhs.theSize;
	head = rhs.head;
	tail = rhs.tail;
	rhs.theSize = 0;
	rhs.head = nullptr;
	rhs.tail = nullptr;
}
template <typename T>
List < T >:: List(int num, const T& val)
{
	init();
	for(int i=0; i<num;i++)
		push_back(val);
}
template <typename T>
List < T >::List(const_iterator start, const_iterator end)
{
	init();
	const_iterator temp = start;
	while (temp != end)
	{
		push_back(temp.retrieve());
		temp++;
	}
}
template <typename T>
List < T >::List (std::initializer_list<T> iList)
{
	init();
	typename std::initializer_list<T>::iterator itr = iList.begin();
	for (auto & x : iList)
		push_back(*itr++);
}
template <typename T>
List < T >::~List()
{
	clear();
	delete head;
	delete tail;
}
template <typename T>
const typename List<T>::List& List < T >:: operator=(const List &rhs)
{
	List copy = rhs;
	std::swap( *this, copy);
	return *this;
}
template <typename T>
typename List<T>::List & List < T >:: operator=(List && rhs)
{
	std::swap(theSize,rhs.theSize);
	std::swap(head,rhs.head );
	std::swap(tail,rhs.tail);
	return *this;
}
template <typename T>
typename List<T>::List& List < T >:: operator= (std::initializer_list<T> iList)
{
	clear();
	typename std::initializer_list<T>::iterator itr = iList.begin();
	for (auto & x : iList)
		push_back(*itr++);
	return *this;
}
template <typename T>
int List < T >:: size() const
{
	return theSize;
}
template <typename T>
bool List < T >:: empty() const
{
	if (theSize == 0)
		return 1;
	else
		return 0;
}
template <typename T>
void List < T >:: clear()
{
	while(!empty())
		pop_front();
}
template <typename T>
void List < T >:: reverse()
{
	Node *current;
	current = head;
	head = tail;
	tail = current;

	for (int i=0;i<theSize+2;i++)
	{
		std::swap(current->prev,current->next);
		current = current->prev;
	}
}
template <typename T>
T& List < T >:: front()
{
	return *begin();
}
template <typename T>
const T& List < T >:: front() const
{
	return *begin();
}
template <typename T>
T& List < T >:: back()
{
	return *--end();
}
template <typename T>
const T& List < T >:: back() const
{
	return *--end();
}
template <typename T>
void List < T >:: push_front(const T & val)
{
	insert(begin(), val);
}
template <typename T>
void List < T >:: push_front(T && val)
{
	insert(begin(), std::move(val));
}
template <typename T>
void List < T >:: push_back(const T & val)
{
	insert(end(), val);
}
template <typename T>
void List < T >:: push_back(T && val)
{
	insert(end(), std::move(val));
}
template <typename T>
void List < T >:: pop_front()
{
	erase(begin());
}
template <typename T>
void List < T >:: pop_back()
{
	erase(--end());
}
template <typename T>
void List < T >:: remove(const T &val)
{
	auto itr = begin();
	while(itr != end())
	{
		if (itr.retrieve() == val)
			itr = erase(itr);
		else
			++itr;
	}
}
template <typename T>
template <typename PREDICATE>
void List < T >:: remove_if(PREDICATE pred)
{
	auto itr = begin();
	while(itr != end())
	{
		if (pred(itr.retrieve()) == 1)
			itr = erase(itr);
		else
			++itr;
	}
}
template <typename T>
void List < T >:: print(std::ostream& os, char ofc) const
{
	auto itr = begin();
	for (int i=0;i<theSize;i++)
	{
		os << itr.retrieve() << ofc;
		++itr;
	}
}
template <typename T>
typename List<T>::iterator List < T >:: begin()
{
	return head->next;
}
template <typename T>
typename List<T>::const_iterator List < T >:: begin() const
{
	return head->next;
}
template <typename T>
typename List<T>::iterator List < T >:: end()
{
	return tail;
}
template <typename T>
typename List<T>::const_iterator List < T >:: end() const
{
	return tail;
}
template <typename T>
typename List<T>::iterator List < T >:: insert(iterator itr, const T& val)
{
	Node *p = itr.current;
	theSize++;
	return { p->prev = p->prev->next = new Node{ val, p->prev, p } };
}
template <typename T>
typename List<T>::iterator List < T >:: insert(iterator itr, T && val)
{
	Node *p = itr.current;
	theSize++;
	return { p->prev = p->prev->next = new Node{ std::move( val ), p->prev, p } };
}
template <typename T>
typename List<T>::iterator List < T >:: erase(iterator itr)
{
	Node *p = itr.current;
	iterator retVal{ p->next };
	p->prev->next = p->next;
	p->next->prev = p-> prev;
	delete p;
	theSize--;
	return retVal;
}
template <typename T>
typename List<T>::iterator List < T >:: erase(iterator start, iterator end)
{
	for (iterator itr = start; itr != end;)
		itr = erase(itr);
	return end;
}
template <typename T>
void List < T >:: init()
{
	theSize = 0;
	head = new Node;
	tail = new Node;
	head->next = tail;
	tail->prev = head;
}
template <typename T>
bool operator==(const List<T> & lhs, const List<T> &rhs)
{
	if (lhs.size() != rhs.size())
		return 0;
	auto Litr = lhs.begin();
	auto Ritr = rhs.begin();
	for (int i=0; i<lhs.size();i++)
	{
		if (*Litr != *Ritr)
			return 0;
		++Litr;
		++Ritr;
	}
	return 1;
}
template <typename T>
bool operator!=(const List<T> & lhs, const List<T> &rhs)
{
	if (lhs == rhs)
		return 0;
	else
		return 1;
}
template <typename T>
std::ostream & operator<<(std::ostream &os, const List<T> &l)
{
	l.print(os);
	return os;
}
