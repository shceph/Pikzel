#include <memory>
#include <utility>
#include <vector>

namespace Pikzel
{
template <typename T> class Tree
{
  public:
    using Node = Tree<T>;

    Tree(Tree&&) = delete;
    Tree(const Tree&) = delete;
    auto operator=(const Tree&) -> Tree& = delete;
    auto operator=(Tree&&) -> Tree& = delete;
    ~Tree() = default;

    template <typename... Args>
    explicit Tree(Args&&... args)
        : mData{std::make_unique<T>(std::forward<Args>(args)...)}
    {
    }

    template <typename U>
    explicit Tree(U&& elem) : mData{std::make_unique<T>(std::forward<U>(elem))}
    {
    }

    template <typename... Args> auto AddChild(Args&&... args) -> Node&
    {
        mChildren.push_back(
            std::unique_ptr<Node>{new Node{this, std::forward<Args>(args)...}});
        mChildLastUsedIndex = mChildren.size() - 1;
        return *mChildren.back().get();
    }

    auto GetParent() const -> Node* { return mParent; }
    auto GetData() const -> const T& { return *mData; }
    auto GetChildren() const -> std::vector<std::unique_ptr<Node>>&
    {
        return mChildren;
    }
    auto GetChildren() -> std::vector<std::unique_ptr<Node>>&
    {
        return mChildren;
    }
    [[nodiscard]] auto GetLastUsedNodeIndex() const -> std::size_t
    {
        return mChildLastUsedIndex;
    }

  private:
    template <typename... Args>
    explicit Tree(Node* parent, Args&&... args)
        : mParent{parent},
          mData(std::make_unique<T>(std::forward<Args>(args)...))
    {
    }

    Node* mParent{nullptr};
    std::unique_ptr<T> mData;
    std::vector<std::unique_ptr<Node>> mChildren;
    std::size_t mChildLastUsedIndex{0};
};
} // namespace Pikzel
