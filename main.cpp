#include <iostream>
#include <vector>
#include <functional>
#include <memory>
#include <random>
#include <time.h>
using namespace std;


class Node {
public:
    typedef long long   type;
    typedef long long   prior_type;
    typedef size_t      size_type;
    typedef Node*       node_ptr;
    typedef mt19937     randomizer;

private:
    static const type _INF = 1e15;
    static randomizer _getRandomNumber;

    size_type _subtreeNodes;
    prior_type _prior;

    node_ptr _l;
    node_ptr _r;

    type _val;
    type _lowest;
    type _highest;
    type _sum;

    bool _isIncreasing;
    bool _isDecreasing;

    bool _toReverse;

    void _baseUpdate(node_ptr t) {
        if (!t)
            return;
        t->_apply();
        _subtreeNodes += t->_subtreeNodes;
        _lowest = std::min(_lowest, t->_lowest);
        _highest = std::max(_highest, t->_highest);
        _sum += t->_sum;
        _isIncreasing &= t->_isIncreasing;
        _isDecreasing &= t->_isDecreasing;
    }

    void _reverse() {
        _toReverse ^= true;
    }

    void _apply() {
        if (_toReverse) {
            std::swap(_l, _r);
            std::swap(_isIncreasing, _isDecreasing);
            if (_l) _l->_reverse();
            if (_r) _r->_reverse();
            _toReverse = false;
        }

        switch (_push.pushType) {
            case Push::Type::no:
                break;
            case Push::Type::set:
                _val = _lowest = _highest = _push.val;
                _sum = _push.val * _subtreeNodes;
                _isIncreasing = _isDecreasing = true;
                break;
            case Push::Type::add:
                _val += _push.val;
                _lowest += _push.val;
                _highest += _push.val;
                _sum += _push.val * _subtreeNodes;
                break;
        }
        if (_l) addPush(_l, _push);
        if (_r) addPush(_r, _push);
        _push = Push();
    }

    void _update() {
        _subtreeNodes = 1;
        _lowest = _highest = _sum = _val;
        _isIncreasing = _isDecreasing = true;

        if (_l) {
            _baseUpdate(_l);
            if (_l->_highest > _val)
                _isIncreasing = false;
            if (_l->_lowest < _val)
                _isDecreasing = false;
        }
        if (_r) {
            _baseUpdate(_r);
            if (_r->_lowest < _val)
                _isIncreasing = false;
            if (_r->_highest > _val)
                _isDecreasing = false;
        }
    }

    size_type _getIndex() const {
        size_type ind = 0;
        if (_l)
            ind += _l->_subtreeNodes;
        return ind;
    }

    static node_ptr _mergeTwo(node_ptr l, node_ptr r) {
        if (l) l->_apply();
        if (r) r->_apply();
        if (!l) return r;
        if (!r) return l;

        if (l->_prior > r->_prior) {
            node_ptr tmp = _mergeTwo(l->_r, r);
            l->_r = tmp;
            l->_update();
            return l;
        } else {
            node_ptr tmp = _mergeTwo(l, r->_l);
            r->_l = tmp;
            r->_update();
            return r;
        }
    }

public:
    struct Push {
        enum class Type {
            no, set, add
        };
        Type pushType;
        type val;

        Push() : pushType(Type::no) {}
        Push(Type pushType, type val) :
                pushType(pushType),
                val(val) {}
    };

private:
    Push _push;

public:
    static void addPush(node_ptr t, Push newPush) {
        if (!t)
            return;
        if (newPush.pushType == Push::Type::no)
            return;
        if (t->_push.pushType == Push::Type::no || newPush.pushType == Push::Type::set) {
            t->_push = newPush;
            return;
        }
        t->_push.val += newPush.val;
    }

    explicit Node(type v = type()) :
            _subtreeNodes(1),
            _prior(_getRandomNumber()),
            _l(nullptr),
            _r(nullptr),
            _isIncreasing(true),
            _isDecreasing(true),
            _toReverse(false),
            _val(v),
            _lowest(v),
            _highest(v),
            _sum(v) {}

    static void reverse(node_ptr t) {
        if (!t)
            return;
        t->_reverse();
    }

    static size_type size(const node_ptr t) {
        return (!t ? 0 : t->_subtreeNodes);
    }

    typedef std::function<bool(const node_ptr)> condition_type;

    static std::pair<node_ptr, node_ptr> splitByCondition(node_ptr t, const condition_type& condition) {
        if (!t) return {nullptr, nullptr};
        t->_apply();
        if (t->_l) t->_l->_apply();
        if (t->_r) t->_r->_apply();

        if (condition(t)) {
            auto tmp = splitByCondition(t->_l, condition);
            t->_l = tmp.second;
            t->_update();
            return std::make_pair(tmp.first, t);
        } else {
            auto tmp = splitByCondition(t->_r, condition);
            t->_r = tmp.first;
            t->_update();
            return std::make_pair(t, tmp.second);
        }
    }

    static std::pair<node_ptr, node_ptr> split(node_ptr t, size_type index) {
        condition_type cnd = [&index](const node_ptr t) {
            if (t->_getIndex() >= index)
                return true;
            index -= t->_getIndex() + 1;
            return false;
        };
        return splitByCondition(t, cnd);
    }

    static size_type findByCondition(node_ptr t, const condition_type& condition) {
        if (!t) return 0;
        t->_apply();

        size_type retVal;
        if (condition(t))
            retVal = findByCondition(t->_l, condition);
        else
            retVal = findByCondition(t->_r, condition) + t->_getIndex() + 1;
        return retVal;
    }

    static void iterate(node_ptr t, std::vector<type>& tmp) {
        if (t) t->_apply();
        if (!t) return;

        iterate(t->_l, tmp);
        tmp.push_back(t->_val);
        iterate(t->_r, tmp);
    }

    static std::vector<type> toVector(node_ptr t) {
        std::vector<type> tmp;
        iterate(t, tmp);
        return tmp;
    }

    static void insert(node_ptr& t, type val, size_type pos) {
        auto newNode = new Node(val);
        auto tmp = split(t, pos);
        t = merge(tmp.first, merge(newNode, tmp.second));
    }

    static node_ptr merge() {
        return nullptr;
    }

    static node_ptr merge(node_ptr l) {
        return l;
    }

    template<typename... Nodes>
    static node_ptr merge(node_ptr l, Nodes... rest) {
        return _mergeTwo(l, merge(rest...));
    }

    static type getSum(const node_ptr t) {
        if (!t)
            return type();
        return t->_sum;
    }

    static type getVal(const node_ptr t) {
        if (!t)
            return type();
        return t->_val;
    }

    typedef std::function<void(node_ptr&)> operation_type;

    static void operationOnSubsegment(node_ptr& t, size_type l, size_type r, const operation_type& operation) {
        auto x = Node::split(t, l);
        auto y = Node::split(x.second, r - l + 1);
        operation(y.first);
        t = Node::merge(x.first, y.first, y.second);
    }

    static void erase(node_ptr& t, size_type pos, size_type count = 1) {
        operationOnSubsegment(t, pos, pos + count - 1, [](node_ptr& t) {
            delete t;
            t = nullptr;
        });
    }

    static node_ptr getNode(node_ptr& t, size_type pos) {
        node_ptr node;
        operationOnSubsegment(t, pos, pos, [&node](node_ptr& t){
            node = t;
        });
        return node;
    }

    static condition_type generateMonotonyCondition(type& vall, bool isIncreasingCnd) {
        condition_type innerSplitCnd;
        if (isIncreasingCnd) {
            vall = Node::_INF;
            innerSplitCnd = [&vall](const node_ptr t) -> bool {
                return ((!t->_r && t->_val <= vall) ||
                        (t->_r && t->_r->_isIncreasing && t->_val <= t->_r->_lowest && t->_r->_highest <= vall));
            };
        } else {
            vall = -Node::_INF;
            innerSplitCnd = [&vall](const node_ptr t) -> bool {
                return ((!t->_r && t->_val >= vall) ||
                        (t->_r && t->_r->_isDecreasing && t->_val >= t->_r->_highest && t->_r->_lowest >= vall));
            };
        }

        return [&vall, innerSplitCnd](const node_ptr t) {
            bool ans = innerSplitCnd(t);
            if (ans)
                vall = t->_val;
            return ans;
        };
    }

    ~Node() {
        delete _l;
        delete _r;
    }
};

Node::randomizer Node::_getRandomNumber = randomizer(time(0));

typedef Node::node_ptr Tree;


class Task {
public:
    class Operation {
    public:
        enum class type;

        class Base {
        public:
            Task* owner;
            virtual void handle() const = 0;
        };

        class Sum;
        class Insert;
        class Remove;
        class SubsegBase;
        class Subsegset;
        class Subsegadd;
        class PermutationBase;
        class NextPermutation;
        class PrevPermutation;
    };

private:
    static std::unique_ptr<Task> _ptr;
    Task() :
            _tree(nullptr) {}

    Tree _tree;
    std::vector<std::unique_ptr<Operation::Base>> _queries;

    typedef std::vector<long long> result_type;
    result_type _result;

public:
    static Task& get() {
        if (!_ptr)
            _ptr.reset(new Task());
        return *_ptr;
    }

    ~Task() {
        delete _tree;
    }

    Tree& getTree() {
        return _tree;
    }

    void addQuery(Operation::Base* operation) {
        operation->owner = this;
        _queries.emplace_back(operation);
    }

    void runQueries() const {
        for (auto& query : _queries)
            query->handle();
    }

    result_type& getResult() {
        return _result;
    }
};

std::unique_ptr<Task> Task::_ptr = nullptr;

enum class Task::Operation::type {
    subsegsum = 1,
    insert,
    remove,
    subsegset,
    subsegadd,
    next_permutation,
    prev_permutation
};

class Task::Operation::Sum : public Base {
    size_t _l, _r;

public:
    Sum(size_t l, size_t r) :
            _l(l),
            _r(r) {}

    void handle() const override {
        Node::operationOnSubsegment(owner->getTree(), _l, _r, [this](Tree& t) {
            owner->getResult().push_back(Node::getSum(t));
        });
    }
};

class Task::Operation::Insert : public Base {
    long long _x;
    size_t _at;

public:
    Insert(long long x, size_t at) :
            _x(x),
            _at(at) {}

    void handle() const override {
        Node::insert(owner->getTree(), _x, _at);
    }
};

class Task::Operation::Remove : public Base {
    long long _pos;

public:
    Remove(long long pos) :
            _pos(pos) {}

    void handle() const override {
        Node::erase(owner->getTree(), _pos);
    }
};

class Task::Operation::SubsegBase : public Base {
    long long _x;
    size_t _l, _r;

protected:
    enum class type {
        set, add
    };
    type tp;

public:
    SubsegBase(long long x, size_t l, size_t r) :
            _x(x),
            _l(l),
            _r(r) {}

    void handle() const override {
        Node::operationOnSubsegment(owner->getTree(), _l, _r, [this](Tree& t) {
            Node::Push::Type opertype = (tp == type::set ?
                                         Node::Push::Type::set :
                                         Node::Push::Type::add);
            Node::addPush(t, Node::Push(opertype, _x));
        });
    }
};

class Task::Operation::Subsegset : public SubsegBase {
public:
    Subsegset(long long x, size_t l, size_t r) :
            SubsegBase(x, l, r) {
        tp = type::set;
    }
};

class Task::Operation::Subsegadd : public SubsegBase {
public:
    Subsegadd(long long x, size_t l, size_t r) :
            SubsegBase(x, l, r) {
        tp = type::add;
    }
};

class Task::Operation::PermutationBase : public Base {
    size_t _l, _r;

protected:
    enum class type {
        next, prev
    };
    type tp;

public:
    PermutationBase(size_t l, size_t r) :
            _l(l),
            _r(r) {}

    void handle() const override {
        Node::operationOnSubsegment(owner->getTree(), _l, _r, [this](Tree& t) {
            long long vall;
            auto splitCnd = Node::generateMonotonyCondition(vall, tp == type::prev);
            auto z = Node::splitByCondition(t, splitCnd);

            if (!z.first) {
                Node::reverse(z.second);
                t = Node::merge(z.second);
                return;
            }

            long long val = Node::getVal(Node::getNode(z.first, Node::size(z.first) - 1));

            std::function<bool(const Tree)> split2cnd;
            if (tp == type::prev)
                split2cnd = [val](const Tree t) { return val <= Node::getVal(t); };
            else
                split2cnd = [val](const Tree t) { return val >= Node::getVal(t); };

            size_t pos = Node::findByCondition(z.second, split2cnd) - 1;
            long long numberatpos = Node::getVal(Node::getNode(z.second, pos));

            Node::erase(z.first, Node::size(z.first) - 1);
            Node::insert(z.first, numberatpos, Node::size(z.first));
            Node::erase(z.second, pos);
            Node::insert(z.second, val, pos);
            Node::reverse(z.second);

            t = Node::merge(z.first, z.second);
        });
    }
};

class Task::Operation::NextPermutation : public PermutationBase {
public:
    NextPermutation(size_t l, size_t r) :
            PermutationBase(l, r) {
        tp = type::next;
    }
};

class Task::Operation::PrevPermutation : public PermutationBase {
public:
    PrevPermutation(size_t l, size_t r) :
            PermutationBase(l, r) {
        tp = type::prev;
    }
};


template<typename T>
void printIterable(const T& a, const char* delimiter, std::ostream& cout = std::cout) {
    size_t position = 0;
    for (auto i : a)
        cout << i << (++position == a.size() ? "" : delimiter);
}


void get(std::istream& cin = std::cin) {
    Tree& t = Task::get().getTree();

    size_t n;
    cin >> n;
    for (size_t i = 0; i < n; ++i) {
        long long x;
        std::cin >> x;
        Node::insert(t, x, Node::size(t));
    }

    Task& task = Task::get();

    size_t q;
    cin >> q;
    for (size_t i = 0; i < q; ++i) {
        int itype, a, b, c;
        cin >> itype;
        switch (static_cast<Task::Operation::type>(itype)) {
            case Task::Operation::type::subsegsum:
                cin >> a >> b;
                task.addQuery(new Task::Operation::Sum(a, b));
                break;
            case Task::Operation::type::insert:
                cin >> a >> b;
                task.addQuery(new Task::Operation::Insert(a, b));
                break;
            case Task::Operation::type::remove:
                cin >> a;
                task.addQuery(new Task::Operation::Remove(a));
                break;
            case Task::Operation::type::subsegset:
                cin >> a >> b >> c;
                task.addQuery(new Task::Operation::Subsegset(a, b, c));
                break;
            case Task::Operation::type::subsegadd:
                cin >> a >> b >> c;
                task.addQuery(new Task::Operation::Subsegadd(a, b, c));
                break;
            case Task::Operation::type::next_permutation:
                cin >> a >> b;
                task.addQuery(new Task::Operation::NextPermutation(a, b));
                break;
            case Task::Operation::type::prev_permutation:
                cin >> a >> b;
                task.addQuery(new Task::Operation::PrevPermutation(a, b));
                break;
        }
    }
}

void handle() {
    Task::get().runQueries();
}

void print(std::ostream& cout = std::cout) {
    printIterable(Task::get().getResult(), "\n", cout);
    cout << '\n';
    printIterable(Node::toVector(Task::get().getTree()), " ",  cout);
    cout << endl;
}


void solve() {
    get();
    handle();
    print();
}

int main() {
    freopen("input.txt", "r", stdin);
    freopen("output.txt", "w", stdout);
    std::ios::sync_with_stdio(false);

    solve();

    return 0;
}
