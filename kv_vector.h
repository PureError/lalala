#include <utility>
#include <algorithm>
#include <vector>

namespace ytc
{
    template<class V, class C>
    struct kv_obj_cmp : C
    {
        typedef typename C::first_argument_type key_t;
        typedef std::pair<key_t, V> object_t;
        typedef kv_obj_cmp<V, C> my_type;

        kv_obj_cmp()/*= default*/ {}
        kv_obj_cmp(const C& h) : C(h) {}
        bool operator()(const object_t& obj, const key_t& key) const
        {
            return C::operator()(obj.first, key);
        }

        bool operator()(const object_t& lhs, const object_t& rhs) const 
        {
            return C::operator()(lhs.first, rhs.first);
        }

        bool operator()(const key_t& key, const object_t& obj) const
        {
            return C::operator()(key, obj.first);
        }
    };

    template<class K, class V, 
    class C = std::less<K>,
    class A = std::allocator<std::pair<K, V>>>
    class kv_vector : private std::vector<std::pair<K, V>, A>
        , private kv_obj_cmp<V, C>
    {
        typedef std::vector<std::pair<K, V>, A> super_t;
        typedef kv_obj_cmp<V, C> cmp_handler_t;
    public:
        typedef kv_vector<K, V, C, A> my_type;
        typedef K key_type;
        typedef V mapped_type;
        typedef typename super_t::value_type value_type;
        typedef C key_compare;
        typedef cmp_handler_t value_compare;
        typedef typename super_t::allocator_type allocator_type;
        typedef typename super_t::reference reference;
        typedef typename super_t::const_reference const_reference;
        typedef typename super_t::pointer pointer;
        typedef typename super_t::const_pointer const_pointer;
        typedef typename super_t::iterator iterator;
        typedef typename super_t::const_iterator const_iterator;
        typedef typename super_t::reverse_iterator reverse_iterator;
        typedef typename super_t::const_reverse_iterator const_reverse_iterator;
        typedef typename super_t::difference_type difference_type;
        typedef typename super_t::size_type size_type;
        typedef std::pair<iterator, iterator> range_t;
        typedef std::pair<const_iterator, const_iterator> crange_t;
    public:
        explicit kv_vector(const key_compare& comp = key_compare(),
            const allocator_type& alloc = allocator_type())
            : cmp_handler_t(comp), super_t(alloc)
        {

        }

        template <class InputIterator>
        kv_vector(InputIterator first, InputIterator last,
            const key_compare& comp = key_compare(),
            const allocator_type& alloc = allocator_type())
            : cmp_handler_t(comp), super_t(first, last, alloc)
        {
			arrange();
        }


        kv_vector(const my_type& v)
            : cmp_handler_t(v), super_t(v)
        {
        }

        ~kv_vector() {}

        my_type& operator=(const my_type& v)
        {
            if (&v != this)
            {
                my_type t(v);
                swap(t);
            }
            return *this;
        }


        iterator begin() { return super_t::begin(); }
        iterator end() { return super_t::end(); }
        const_iterator begin() const { return super_t::begin(); }
        const_iterator end() const { return super_t::end(); }
        reverse_iterator rbegin() { return super_t::rbegin(); }
        const_reverse_iterator rbegin() const { return super_t::rbegin(); }
        bool empty() const { return super_t::empty(); }
        size_type size() const { return super_t::size(); }
        size_type max_size() const { return super_t::max_size(); }
        allocator_type get_allocator() const { return super_t::get_allocator(); }

		void arrange()
		{
			std::sort(begin(), end(), static_cast<cmp_handler_t&>(*this));
		}

        void reserve(size_type n)
        {
            super_t::reserve(n);
        }

        std::pair<iterator, bool> insert(const_reference val)
        {
            iterator position = lower_bound(val.first);
            bool exist = position != this->end() && !(*this)(val.first, *position);
            return std::make_pair(exist ? position : super_t::insert(position, val), !exist);
        }

        mapped_type& operator[](const key_type& key)
        {
            return (*((this->insert(std::make_pair(key, mapped_type()))).first)).second;
        }

        iterator insert(iterator position, const_reference val)
        {
            if (position != this->end() &&
                (*this)(*position, val) && 
                ((position + 1) == this->end() || !(*this)(val, *(position + 1) && (*this)(*(position + 1), val))))
            {
                return super_t::insert(position, val);
            }
            return this->insert(val).first;
        }

        template<class InputIterator>
        void insert(InputIterator first, InputIterator last)
        {
            while(first != last) this->insert(*first++);
        }

        void erase(iterator position) { super_t::erase(position); }
        size_type erase(const key_type& key) 
        {
            iterator position = this->find(key);
            if (position != this->end())
            {
                this->erase(position);
                return size_type(1);
            }
            return size_type();
        }

        void erase(iterator first, iterator last)
        {
            super_t::erase(first, last);
        }

        void swap(my_type& v)
        {
            super_t::swap(static_cast<super_t&>(v));
            cmp_handler_t& lhs = *this;
            cmp_handler_t& rhs = v;
            std::swap(lhs, rhs);
        }

        iterator find(const key_type& key)
        {
            iterator position = lower_bound(key);
            return position != this->end() ? 
                ((*this)(key, *position) ? this->end() : position) :
                position;
        }

        const_iterator find(const key_type& key) const 
        {
            const_iterator position = lower_bound(key);
            return position != this->end() ?
                ((*this)(key, *position) ? this->end() : position) :
                position;
        }

        size_type count(const key_type& k) const
        { 
            return size_type(find(k) != end() ? 1 : 0); 
        }

        iterator lower_bound(const key_type& key)
        {
            return std::lower_bound(this->begin(), this->end(), key, static_cast<cmp_handler_t&>(*this));
        }

        const_iterator lower_bound(const key_type& key) const
        {
            return std::lower_bound(this->begin(), this->end(), key, static_cast<const cmp_handler_t&>(*this));
        }

        iterator upper_bound(const key_type& key)
        {
            return std::upper_bound(this->begin(), this->end(), key, static_cast<cmp_handler_t&>(*this));
        }

        const_iterator upper_bound(const key_type& key) const
        {
            return std::upper_bound(this->begin(), this->end(), key, static_cast<const cmp_handler_t&>(*this));
        }

        range_t equal_range(const key_type& key)
        {
            return std::equal_range(this->begin(), this->end(), key, static_cast<cmp_handler_t&>(*this));
        }

        crange_t equal_range(const key_type& key) const
        {
            return std::equal_range(this->begin(), this->end(), key, static_cast<const cmp_handler_t&>(*this));
        }

        void clear() { super_t::clear(); }

        key_compare key_comp() const
        {
            return *this;
        }

        value_compare value_comp() const
        {
            const key_compare& comp = *this;
            return value_compare(comp);
        }

    };
}
