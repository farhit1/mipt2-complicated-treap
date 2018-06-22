#include <iostream>
#include <vector>
#include <functional>
#include <memory>
using namespace std;


class Node {
public:
    typedef long long   type;
    typedef long long   prior_type;
    typedef size_t      size_type;
    typedef Node*       node_ptr;

private:
    static const long long INF = 1e15;

    size_type subtreeNodes;
    prior_type prior;

    node_ptr l;
    node_ptr r;

    type val;
    type lowest;
    type highest;
    type sum;

    bool isIncreasing;
    bool isDecreasing;

    bool toReverse;

    void baseUpdate(node_ptr child) {
        if (!child) return;
        child->apply();
        subtreeNodes += child->subtreeNodes;
        lowest = std::min(lowest, child->lowest);
        highest = std::max(highest, child->highest);
        sum += child->sum;
        isIncreasing &= child->isIncreasing;
        isDecreasing &= child->isDecreasing;
    }

    void reverse() {
        toReverse ^= true;
    }

    void apply() {
        if (toReverse) {
            std::swap(l, r);
            std::swap(isIncreasing, isDecreasing);
            if (l) l->reverse();
            if (r) r->reverse();
            toReverse = false;
        }

        switch (push.pushType) {
            case Push::Type::no:
                break;
            case Push::Type::set:
                val = lowest = highest = push.val;
                sum = push.val * subtreeNodes;
                isIncreasing = isDecreasing = true;
                break;
            case Push::Type::add:
                val += push.val;
                lowest += push.val;
                highest += push.val;
                sum += push.val * subtreeNodes;
                break;
        }
        if (l) addPush(l, push);
        if (r) addPush(r, push);
        push = Push();
    }

    void update() {
        subtreeNodes = 1;
        lowest = highest = sum = val;
        isIncreasing = isDecreasing = true;

        if (l) {
            baseUpdate(l);
            if (l->highest > val)
                isIncreasing = false;
            if (l->lowest < val)
                isDecreasing = false;
        }
        if (r) {
            baseUpdate(r);
            if (r->lowest < val)
                isIncreasing = false;
            if (r->highest > val)
                isDecreasing = false;
        }
    }

    size_type getIndex() const {
        size_type ind = 0;
        if (l)
            ind += l->subtreeNodes;
        return ind;
    }

    static node_ptr mergeTwo(node_ptr l, node_ptr r) {
        if (l) l->apply();
        if (r) r->apply();
        if (!l) return r;
        if (!r) return l;

        if (l->prior > r->prior) {
            node_ptr tmp = mergeTwo(l->r, r);
            l->r = tmp;
            l->update();
            return l;
        } else {
            node_ptr tmp = mergeTwo(l, r->l);
            r->l = tmp;
            r->update();
            return r;
        }
    }

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
    Push push;

    static void addPush(node_ptr t, Push newPush) {
        if (!t)
            return;
        if (newPush.pushType == Push::Type::no)
            return;
        if (t->push.pushType == Push::Type::no || newPush.pushType == Push::Type::set) {
            t->push = newPush;
            return;
        }
        t->push.val += newPush.val;
    }

    friend class Operation;

public:
    Node() :
            subtreeNodes(1),
            prior(rand()),
            l(nullptr),
            r(nullptr),
            isIncreasing(true),
            isDecreasing(true),
            toReverse(false) {}

    explicit Node(type v) :
            Node() {
        val = v;
        lowest = v;
        highest = v;
        sum = v;
    }

    static void reverse(node_ptr t) {
        if (!t)
            return;
        t->reverse();
    }

    static size_type size(const node_ptr t) {
        return (!t ? 0 : t->subtreeNodes);
    }

    typedef std::function<bool(const node_ptr)> condition_type;

    static std::pair<node_ptr, node_ptr> splitByCondition(node_ptr t, const condition_type& condition) {
        if (!t) return {nullptr, nullptr};
        t->apply();
        if (t->l) t->l->apply();
        if (t->r) t->r->apply();

        if (condition(t)) {
            auto tmp = splitByCondition(t->l, condition);
            t->l = tmp.second;
            t->update();
            return std::make_pair(tmp.first, t);
        } else {
            auto tmp = splitByCondition(t->r, condition);
            t->r = tmp.first;
            t->update();
            return std::make_pair(t, tmp.second);
        }
    }

    static std::pair<node_ptr, node_ptr> split(node_ptr t, size_type index) {
        condition_type cnd = [&index](const node_ptr t) {
            if (t->getIndex() >= index)
                return true;
            index -= t->getIndex() + 1;
            return false;
        };
        return splitByCondition(t, cnd);
    }

    static size_type findByCondition(node_ptr t, const condition_type& condition) {
        if (!t) return 0;
        t->apply();

        size_type retVal;
        if (condition(t))
            retVal = findByCondition(t->l, condition);
        else
            retVal = findByCondition(t->r, condition) + t->getIndex() + 1;
        return retVal;
    }

    static void iterate(node_ptr t, std::vector<type>& tmp) {
        if (t) t->apply();
        if (!t) return;

        iterate(t->l, tmp);
        tmp.push_back(t->val);
        iterate(t->r, tmp);
    }

    static std::vector<type> toVector(node_ptr t) {
        std::vector<type> tmp;
        iterate(t, tmp);
        return tmp;
    }

    static node_ptr getNode(node_ptr& t, size_type pos) {
        auto tmp1 = split(t, pos);
        auto tmp2 = split(tmp1.second, 1);
        node_ptr node = tmp2.first;
        t = merge(tmp1.first, merge(tmp2.first, tmp2.second));
        return node;
    }

    static void erase(node_ptr& t, size_type pos, size_type count = 1) {
        auto tmp1 = split(t, pos);
        auto tmp2 = split(tmp1.second, count);
        delete tmp2.first;
        t = merge(tmp1.first, tmp2.second);
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
        return mergeTwo(l, merge(rest...));
    }

    static type getSum(const node_ptr t) {
        if (!t)
            return type();
        return t->sum;
    }

    static type getVal(const node_ptr t) {
        if (!t)
            return type();
        return t->val;
    }

    ~Node() {
        delete l;
        delete r;
    }
};


class Operation {
public:
    enum class type;
    class Base;
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

class Task {
    static std::unique_ptr<Task> ptr;
    Task() :
            t(nullptr) {}

public:
    Node* t;
    std::vector<std::unique_ptr<Operation::Base>> queries;
    std::vector<long long> result;

    static Task& get() {
        if (!ptr)
            ptr.reset(new Task());
        return *ptr;
    }

    ~Task() {
        delete t;
    }
};
std::unique_ptr<Task> Task::ptr = nullptr;


enum class Operation::type {
    subsegsum = 1,
    insert,
    remove,
    subsegset,
    subsegadd,
    next_permutation,
    prev_permutation
};

class Operation::Base {
public:
    virtual void handle() const = 0;
};

class Operation::Sum : public Base {
    size_t l, r;

public:
    Sum(size_t l, size_t r) :
            l(l),
            r(r) {}

    void handle() const override {
        Node*& t = Task::get().t;

        auto x = Node::split(t, l);
        auto y = Node::split(x.second, r - l + 1);
        Task::get().result.push_back(Node::getSum(y.first));
        t = Node::merge(x.first, y.first, y.second);
    }
};

class Operation::Insert : public Base {
    long long x;
    size_t at;

public:
    Insert(long long x, size_t at) :
            x(x),
            at(at) {}

    void handle() const override {
        Node*& t = Task::get().t;

        Node::insert(t, x, at);
    }
};

class Operation::Remove : public Base {
    long long pos;

public:
    Remove(long long pos) :
            pos(pos) {}

    void handle() const override {
        Node*& t = Task::get().t;

        Node::erase(t, pos);
    }
};

class Operation::SubsegBase : public Base {
    long long x;
    size_t l, r;

protected:
    enum class type {
        set, add
    };
    type tp;

public:
    SubsegBase(long long x, size_t l, size_t r) :
            x(x),
            l(l),
            r(r) {}

    void handle() const override {
        Node*& t = Task::get().t;

        auto e = Node::split(t, l);
        auto f = Node::split(e.second, r - l + 1);

        Node::Push::Type opertype = (tp == type::set ?
                                     Node::Push::Type::set :
                                     Node::Push::Type::add);
        Node::addPush(f.first, Node::Push(opertype, x));

        t = Node::merge(e.first, f.first, f.second);
    }
};

class Operation::Subsegset : public SubsegBase {
public:
    Subsegset(long long x, size_t l, size_t r) :
            SubsegBase(x, l, r) {
        tp = type::set;
    }
};

class Operation::Subsegadd : public SubsegBase {
public:
    Subsegadd(long long x, size_t l, size_t r) :
            SubsegBase(x, l, r) {
        tp = type::add;
    }
};

class Operation::PermutationBase : public Base {
    size_t l, r;

protected:
    enum class type {
        next, prev
    };
    type tp;

public:
    PermutationBase(size_t l, size_t r) :
            l(l),
            r(r) {}

    void handle() const override {
        Node*& t = Task::get().t;

        auto x = Node::split(t, l);
        auto y = Node::split(x.second, r - l + 1);

        std::function<bool(const Node*)> splitcnd;
        if (tp == type::prev) {
            long long vall = Node::INF;
            splitcnd = [&vall](const Node* t) {
                bool ans = ((!t->r && t->val <= vall) ||
                            (t->r && t->r->isIncreasing && t->val <= t->r->lowest && t->r->highest <= vall));
                if (ans)
                    vall = t->val;
                return ans;
            };
        } else {
            long long vall = -Node::INF;
            splitcnd = [&vall](const Node* t) {
                bool ans = ((!t->r && t->val >= vall) ||
                            (t->r && t->r->isDecreasing && t->val >= t->r->highest && t->r->lowest >= vall));
                if (ans)
                    vall = t->val;
                return ans;
            };
        }

        auto z = Node::splitByCondition(y.first, splitcnd);

        if (!z.first) {
            Node::reverse(z.second);
            t = Node::merge(x.first, z.second, y.second);
            return;
        }

        long long val = Node::getVal(Node::getNode(z.first, Node::size(z.first) - 1));

        std::function<bool(const Node*)> split2cnd;
        if (tp == type::prev)
            split2cnd = [val](const Node* t) { return val <= t->val; };
        else
            split2cnd = [val](const Node* t) { return val >= t->val; };

        size_t pos = Node::findByCondition(z.second, split2cnd) - 1;
        long long numberatpos = Node::getVal(Node::getNode(z.second, pos));

        Node::erase(z.first, Node::size(z.first) - 1);
        Node::insert(z.first, numberatpos, Node::size(z.first));
        Node::erase(z.second, pos);
        Node::insert(z.second, val, pos);
        Node::reverse(z.second);

        t = Node::merge(x.first, z.first, z.second, y.second);
    }
};

class Operation::NextPermutation : public PermutationBase {
public:
    NextPermutation(size_t l, size_t r) :
            PermutationBase(l, r) {
        tp = type::next;
    }
};

class Operation::PrevPermutation : public PermutationBase {
public:
    PrevPermutation(size_t l, size_t r) :
            PermutationBase(l, r) {
        tp = type::prev;
    }
};


void get(std::istream& cin = std::cin) {
    Node*& t = Task::get().t;

    size_t n;
    cin >> n;
    for (size_t i = 0; i < n; ++i) {
        long long x;
        std::cin >> x;
        Node::insert(t, x, Node::size(t));
    }

    auto& tmp = Task::get().queries;

    size_t q;
    cin >> q;
    for (size_t i = 0; i < q; ++i) {
        int itype, a, b, c;
        cin >> itype;
        switch (static_cast<Operation::type>(itype)) {
            case Operation::type::subsegsum:
                cin >> a >> b;
                tmp.emplace_back(new Operation::Sum(a, b));
                break;
            case Operation::type::insert:
                cin >> a >> b;
                tmp.emplace_back(new Operation::Insert(a, b));
                break;
            case Operation::type::remove:
                cin >> a;
                tmp.emplace_back(new Operation::Remove(a));
                break;
            case Operation::type::subsegset:
                cin >> a >> b >> c;
                tmp.emplace_back(new Operation::Subsegset(a, b, c));
                break;
            case Operation::type::subsegadd:
                cin >> a >> b >> c;
                tmp.emplace_back(new Operation::Subsegadd(a, b, c));
                break;
            case Operation::type::next_permutation:
                cin >> a >> b;
                tmp.emplace_back(new Operation::NextPermutation(a, b));
                break;
            case Operation::type::prev_permutation:
                cin >> a >> b;
                tmp.emplace_back(new Operation::PrevPermutation(a, b));
                break;
        }
    }
}

void handle() {
    for (auto& query : Task::get().queries)
        query->handle();
}

void print() {
    Node*& t = Task::get().t;

    for (auto i : Task::get().result)
        cout << i << "\n";
    auto v = Node::toVector(t);
    for (size_t i = 0; i < v.size(); i++)
        cout << v[i] << (i != v.size() - 1 ? " " : "");
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