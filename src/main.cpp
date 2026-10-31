#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>

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

        auto fields = m_fields;
        if (!fields || !m_gameState.m_isPracticeMode) return;

        int percent = static_cast<int>(m_gameState.m_currentProgress);
        fields->m_attempts++;

        if (percent > fields->m_best) {
            fields->m_best = percent;
        }

        if (!fields->m_goalLabel) {
            fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");
            fields->m_goalLabel->setAnchorPoint({0.f, 1.f});
            fields->m_goalLabel->setScale(0.45f);
            fields->m_goalLabel->setZOrder(999);
            fields->m_goalLabel->setID("session-goals-label");

            auto winSize = CCDirector::get()->getWinSize();
            fields->m_goalLabel->setPosition({10.f, winSize.height - 10.f});

            m_uiLayer->addChild(fields->m_goalLabel);
        }

        std::string text = fmt::format(
            "Goal: {}%
Best: {}%
Attempts: {}",
            fields->m_goal,
            fields->m_best,
            fields->m_attempts
        );

        fields->m_goalLabel->setString(text.c_str());

        if (!fields->m_goalCompleted && percent >= fields->m_goal) {
            fields->m_goalCompleted = true;

            auto popup = geode::Popup<>::create(
                "Goal Complete!",
                fmt::format(
                    "You reached {}%!",
                    fields->m_goal
                )
            );

            popup->m_bNoElasticity = true;
            popup->show();
        }
    }
};
