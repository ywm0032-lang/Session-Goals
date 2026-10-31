#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class GoalPopup : public FLAlertLayer, public FLAlertLayerProtocol {
protected:
    TextInput* m_levelIdInput = nullptr;
    TextInput* m_goalInput = nullptr;

    bool init() {
        if (!FLAlertLayer::init(
            this,
            "Set Goal",
            "",
            "Cancel",
            "",
            340.f,
            false,
            250.f,
            1.f
        )) {
            return false;
        }

        if (!m_mainLayer || !m_buttonMenu) {
            return false;
        }

        this->setBrownBackground();

        float centerX = 170.f;
        float centerY = 125.f;

        auto levelLabel = CCLabelBMFont::create("Level ID", "bigFont.fnt");
        if (levelLabel) {
            levelLabel->setScale(0.5f);
            levelLabel->setPosition({centerX, centerY + 55.f});
            m_mainLayer->addChild(levelLabel);
        }

        m_levelIdInput = TextInput::create(240.f, "Level ID");
        if (m_levelIdInput) {
            m_levelIdInput->setPosition({centerX, centerY + 25.f});
            m_mainLayer->addChild(m_levelIdInput);
        }

        auto goalLabel = CCLabelBMFont::create("Goal Percentage", "bigFont.fnt");
        if (goalLabel) {
            goalLabel->setScale(0.5f);
            goalLabel->setPosition({centerX, centerY - 20.f});
            m_mainLayer->addChild(goalLabel);
        }

        m_goalInput = TextInput::create(240.f, "Goal %");
        if (m_goalInput) {
            m_goalInput->setPosition({centerX, centerY - 50.f});
            m_mainLayer->addChild(m_goalInput);
        }

        auto saveButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save"),
            this,
            menu_selector(GoalPopup::onSave)
        );

        if (saveButton) {
            saveButton->setPosition({centerX, centerY - 95.f});

            auto menu = CCMenu::create();
            if (menu) {
                menu->addChild(saveButton);
                menu->setPosition({0.f, 0.f});
                m_buttonMenu->addChild(menu);
            }
        }

        return true;
    }

    void setBrownBackground() {
        auto oldBackground = m_mainLayer->getChildByType<CCScale9Sprite>(0);

        auto brownBackground = CCScale9Sprite::create("GJ_square04.png");

        if (!brownBackground) {
            return;
        }

        brownBackground->setContentSize({320.f, 230.f});
        brownBackground->setPosition({170.f, 125.f});
        brownBackground->setZOrder(-1);

        m_mainLayer->addChild(brownBackground);

        if (oldBackground) {
            oldBackground->removeFromParent();
        }
    }

    void onSave(CCObject*) {
        if (!m_levelIdInput || !m_goalInput) {
            return;
        }

        std::string levelId = m_levelIdInput->getString();
        std::string goalText = m_goalInput->getString();

        if (levelId.empty() || goalText.empty()) {
            Notification::create(
                "Enter a level ID and goal.",
                NotificationIcon::Warning,
                2.f
            )->show();
            return;
        }

        int goal = std::atoi(goalText.c_str());

        if (goal < 1 || goal > 100) {
            Notification::create(
                "Goal must be between 1% and 100%.",
                NotificationIcon::Warning,
                2.f
            )->show();
            return;
        }

        Mod::get()->setSavedValue("goal-level", levelId);
        Mod::get()->setSavedValue("goal-percent", goal);
        Mod::get()->setSavedValue("goal-attempts", 0);

        Notification::create(
            "Goal saved.",
            NotificationIcon::Success,
            2.f
        )->show();

        this->onBtn1(nullptr);
    }

public:
    static GoalPopup* create() {
        auto result = new GoalPopup();

        if (result && result->init()) {
            result->autorelease();
            return result;
        }

        if (result) {
            delete result;
        }

        return nullptr;
    }
};

class GoalsPlayLayer : public Modify<GoalsPlayLayer, PlayLayer> {
public:
    struct Fields {
        CCLabelBMFont* m_goalLabel = nullptr;
    };

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        PlayLayer::destroyPlayer(player, object);

        int attempts = Mod::get()->getSavedValue<int>("goal-attempts", 0);
        Mod::get()->setSavedValue("goal-attempts", attempts + 1);

        this->updateGoalLabel();
    }

    void updateGoalLabel() {
        std::string levelId = Mod::get()->getSavedValue<std::string>("goal-level", "");
        int goal = Mod::get()->getSavedValue<int>("goal-percent", 0);
        int attempts = Mod::get()->getSavedValue<int>("goal-attempts", 0);

        if (levelId.empty() || goal <= 0) {
            return;
        }

        if (!m_fields->m_goalLabel) {
            m_fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");

            if (!m_fields->m_goalLabel || !m_uiLayer) {
                return;
            }

            m_fields->m_goalLabel->setAnchorPoint({0.5f, 1.f});
            m_fields->m_goalLabel->setScale(0.4f);
            m_fields->m_goalLabel->setZOrder(999);
            m_fields->m_goalLabel->setID("session-goals-label"_spr);

            auto winSize = CCDirector::get()->getWinSize();
            m_fields->m_goalLabel->setPosition({
                winSize.width / 2.f,
                winSize.height - 10.f
            });

            m_uiLayer->addChild(m_fields->m_goalLabel);
        }

        std::string text = fmt::format(
            "Level: {}  Goal: {}%  Attempts: {}",
            levelId,
            goal,
            attempts
        );

        m_fields->m_goalLabel->setString(text.c_str());
    }
};

class GoalsPauseLayer : public Modify<GoalsPauseLayer, PauseLayer> {
public:
    void customSetup() {
        PauseLayer::customSetup();

        auto winSize = CCDirector::get()->getWinSize();

        auto goalsButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Goals"),
            this,
            menu_selector(GoalsPauseLayer::onGoals)
        );

        if (!goalsButton) {
            return;
        }

        goalsButton->setPosition({
            winSize.width - 140.f,
            50.f
        });

        auto menu = CCMenu::create();

        if (!menu) {
            return;
        }

        menu->addChild(goalsButton);
        menu->setPosition({0.f, 0.f});
        menu->setZOrder(999);

        this->addChild(menu);
    }

    void onGoals(CCObject*) {
        GoalPopup::create()->show();
    }
};
