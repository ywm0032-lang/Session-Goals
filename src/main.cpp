#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class GoalsPlayLayer : public Modify<GoalsPlayLayer, PlayLayer> {
public:
    struct Fields {
        int m_goal = 100;
        int m_best = 0;
        int m_attempts = 0;
        bool m_goalCompleted = false;
        CCLabelBMFont* m_goalLabel = nullptr;
    };

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        int percent = static_cast<int>(m_gameState.m_currentProgress);
        m_fields->m_attempts++;

        if (percent > m_fields->m_best) {
            m_fields->m_best = percent;
        }

        if (!m_fields->m_goalLabel) {
            m_fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");
            m_fields->m_goalLabel->setAnchorPoint({0.f, 1.f});
            m_fields->m_goalLabel->setScale(0.45f);
            m_fields->m_goalLabel->setZOrder(999);
            m_fields->m_goalLabel->setID("session-goals-label");

            auto winSize = CCDirector::get()->getWinSize();
            m_fields->m_goalLabel->setPosition({10.f, winSize.height - 10.f});

            m_uiLayer->addChild(m_fields->m_goalLabel);
        }

        std::string text = fmt::format(
            "Goal: {}%  Best: {}%  Attempts: {}",
            m_fields->m_goal,
            m_fields->m_best,
            m_fields->m_attempts
        );

        m_fields->m_goalLabel->setString(text.c_str());

        if (!m_fields->m_goalCompleted && percent >= m_fields->m_goal) {
            m_fields->m_goalCompleted = true;

            Notification::create(
                fmt::format("Goal complete! You reached {}%!", m_fields->m_goal),
                NotificationIcon::Success,
                3.f
            )->show();
        }
    }
};
