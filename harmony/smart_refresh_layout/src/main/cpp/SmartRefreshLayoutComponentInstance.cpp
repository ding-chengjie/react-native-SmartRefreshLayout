#include "SmartRefreshLayoutComponentInstance.h"
#include <folly/dynamic.h>
#include "RNOH/arkui/ArkUINode.h"
#include "RNOH/arkui/ScrollNode.h"
#include "RNOHCorePackage/ComponentInstances/ScrollViewComponentInstance.h"
namespace rnoh {

    SmartRefreshLayoutComponentInstance::SmartRefreshLayoutComponentInstance(Context context)
        : CppComponentInstance(std::move(context)) {

        m_pullToRefreshNode.insertChild(m_headerStackNode, 0);
        m_pullToRefreshNode.insertChild(m_listStackNode, 1);
        m_pullToRefreshNode.setPullToRefreshNodeDelegate(this);
        m_headerStackNode.setHeight(facebook::react::Size({0, 0}));

        ArkUI_NumberValue clipValue[] = {{.u32 = 1}};
        ArkUI_AttributeItem clipItem = {clipValue, sizeof(clipValue) / sizeof(ArkUI_NumberValue)};
        NativeNodeApi::getInstance()->setAttribute(m_headerStackNode.getArkUINodeHandle(), NODE_CLIP, &clipItem);

//         ArkUINodeRegistry::getInstance().registerTouchHandler(&m_pullToRefreshNode, this);
//         NativeNodeApi::getInstance()->registerNodeEvent(m_pullToRefreshNode.getArkUINodeHandle(), NODE_TOUCH_EVENT, 0);
    }

    void SmartRefreshLayoutComponentInstance::onTouchEvent(ArkUI_NodeTouchEvent e) {
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::onTouchEvent> nodeY :" << e.actionTouch.nodeY
                  << "; action:" << e.action;
        
    }

    void SmartRefreshLayoutComponentInstance::insertChild(ComponentInstance::Shared childComponentInstance,
                                                          std::size_t index) {
        CppComponentInstance::insertChild(childComponentInstance, index);
        if (!isHeaderInserted) {
            m_headerStackNode.insertChild(childComponentInstance->getLocalRootArkUINode(), index);

            isHeaderInserted = true;
        } else {
            m_listStackNode.insertChild(childComponentInstance->getLocalRootArkUINode(), index);
        }
    }

    void SmartRefreshLayoutComponentInstance::removeChild(ComponentInstance::Shared childComponentInstance) {
        CppComponentInstance::removeChild(childComponentInstance);
        m_headerStackNode.removeChild(childComponentInstance->getLocalRootArkUINode());
        m_listStackNode.removeChild(childComponentInstance->getLocalRootArkUINode());
    };

    PullToRefreshNode &SmartRefreshLayoutComponentInstance::getLocalRootArkUINode() { return m_pullToRefreshNode; }

    void SmartRefreshLayoutComponentInstance::setParent(ComponentInstance::Shared parent) {
        CppComponentInstance::setParent(parent);
        LOG(INFO) << "[clx] <ssss11 setParent.componentName :" << parent->getContext().componentName
                  << ";tag:" << parent->getContext().tag;
    }
    bool  SmartRefreshLayoutComponentInstance::isComponentTop() {
        std::vector<ComponentInstance::Shared> child = getChildren();
        for (ComponentInstance::Shared c : child) {
            if (c->getContext().componentName == "ScrollView") {
                auto scrollView = std::dynamic_pointer_cast<rnoh::ScrollViewComponentInstance>(c);
                if (scrollView != nullptr) {
                    return scrollView->getLocalRootArkUINode().getScrollOffset().y <= 0;
                }
            }
        }
        return false;
    };


    void SmartRefreshLayoutComponentInstance::setProps(facebook::react::Props::Shared props) {
        ComponentInstance::setProps(props);
        auto refreshProps = std::dynamic_pointer_cast<const facebook::react::SmartRefreshLayoutProps>(props);
        if (refreshProps == nullptr) {
            return;
        }
        // TODO: refreshProps attributes
        headerHeight = refreshProps->headerHeight;

        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> headerHeight:" << headerHeight;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> autoRefresh.refresh:"
                  << refreshProps->autoRefresh.refresh;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> autoRefresh.time:"
                  << refreshProps->autoRefresh.time;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> autoRefresh.refresh:"
                  << refreshProps->autoRefresh.refresh;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> maxDragRate:" << refreshProps->maxDragRate;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> dragRate:" << refreshProps->dragRate;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> enableRefresh:"
                  << refreshProps->enableRefresh;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> pureScroll:" << refreshProps->pureScroll;
        LOG(INFO) << "[clx] <SmartRefreshLayoutComponentInstance::setProps> refreshProps:"
                  << refreshProps->primaryColor;

        m_pullToRefreshNode.setEnableRefresh(refreshProps->enableRefresh);
        m_pullToRefreshNode.setHeaderHeight(refreshProps->headerHeight);
    }

    void SmartRefreshLayoutComponentInstance::setEventEmitter(facebook::react::SharedEventEmitter eventEmitter) {
        ComponentInstance::setEventEmitter(eventEmitter);
        auto smartRefreshLayoutEventEmitter =
            std::dynamic_pointer_cast<const facebook::react::SmartRefreshLayoutEventEmitter>(eventEmitter);
        if (smartRefreshLayoutEventEmitter == nullptr) {
            return;
        }
        m_smartRefreshLayoutEventEmitter = smartRefreshLayoutEventEmitter;
    };

    void SmartRefreshLayoutComponentInstance::onRefresh() { m_smartRefreshLayoutEventEmitter->onRefresh({}); };

    void SmartRefreshLayoutComponentInstance::onHeaderPulling(const float &displayedHeaderHeight) {
        facebook::react::Float percent = displayedHeaderHeight / headerHeight;
        m_smartRefreshLayoutEventEmitter->onHeaderPulling({percent, displayedHeaderHeight, headerHeight});
    };

    void SmartRefreshLayoutComponentInstance::onHeaderReleasing(const float &displayedHeaderHeight) {
        facebook::react::Float percent = displayedHeaderHeight / headerHeight;
        m_smartRefreshLayoutEventEmitter->onHeaderReleasing({percent, displayedHeaderHeight, headerHeight});
    };

    void SmartRefreshLayoutComponentInstance::onHeaderMoving(const float &displayedHeaderHeight) {
        LOG(INFO) << "[clx] <PullToRefreshNode::onActionEnd> onHeaderMoving";
        facebook::react::Float percent = displayedHeaderHeight / headerHeight;
        m_smartRefreshLayoutEventEmitter->onHeaderMoving({percent, displayedHeaderHeight, headerHeight});
    };
    void SmartRefreshLayoutComponentInstance::onPullDownToRefresh() {
        m_smartRefreshLayoutEventEmitter->onPullDownToRefresh({});
    };
    void SmartRefreshLayoutComponentInstance::onReleaseToRefresh() {
        m_smartRefreshLayoutEventEmitter->onReleaseToRefresh({});
    };
    void SmartRefreshLayoutComponentInstance::onHeaderReleased() {
        m_smartRefreshLayoutEventEmitter->onHeaderReleased({});
    }
    void SmartRefreshLayoutComponentInstance::handleCommand(std::string const &commandName,
                                                            folly::dynamic const &args) {
        if (commandName == "finishRefresh" && args.isArray() && args.size() == 2) {
            auto delayed = args[0];
            auto success = args[1];
            if (delayed != INFINITY) {
                if (delayed >= 0) {
                    m_pullToRefreshNode.getPullToRefreshConfigurator().setFinishDelay(delayed.asInt());
                } else {
                    m_pullToRefreshNode.getPullToRefreshConfigurator().setFinishDelay(0);
                }
            } else {
                m_pullToRefreshNode.getPullToRefreshConfigurator().setFinishDelay(500);
            }
            m_pullToRefreshNode.finishRefresh();
        }
    };

} // namespace rnoh
