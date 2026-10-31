#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/ui/Notification.hpp>

using namespace geode::prelude;

class GoalPopup : public geode::Popup<> {
protected:
    TextInput* m_levelIdInput;
    TextInput* m_goalInput;

    bool setup() override {
        this->setTitle("Set Goal");

        auto winSize = CCDirector::get()->getWinSize();

        auto levelLabel = CCLabelBMFont::create("Level ID", "bigFont.fnt");
        levelLabel->setScale(0.5f);
        levelLabel->setPosition({75.f, 115.f});
        m_mainLayer->addChild(levelLabel);

        m_levelIdInput = TextInput::create(180.f, 30.f, "Level ID");
        m_levelIdInput->setPosition({75.f, 90.f});
        m_levelIdInput->setScale(0.8f);
        m_mainLayer->addChild(m_levelIdInput);

        auto goalLabel = CCLabelBMFont::create("Goal %", "bigFont.fnt");
        goalLabel->setScale(0.5f);
        goalLabel->setPosition({75.f, 55.f});
        m_mainLayer->addChild(goalLabel);

        m_goalInput = TextInput::create(180.f, 30.f, "Goal %");
        m_goalInput->setPosition({75.f, 30.f});
        m_goalInput->setScale(0.8f);
        m_mainLayer->addChild(m_goalInput);

        auto saveBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Save"),
            this,
            menu_selector(GoalPopup::onSave)
        );
        saveBtn->setPosition({180.f, 20.f});

        auto menu = CCMenu::create();
        menu->addChild(saveBtn);
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);

        return true;
    }

    void onSave(CCObject*) {
        auto levelId = m_levelIdInput->getString();
        auto goalText = m_goalInput->getString();

        if (levelId.empty() || goalText.empty()) {
            Notification::create("Enter a level ID and goal.", NotificationIcon::Warning, 2.f)->show();
            return;
        }

        int goal = std::atoi(goalText.c_str());

        if (goal < 1 || goal > 100) {
            Notification::create("Goal must be 1-100%.", NotificationIcon::Warning, 2.f)->show();
            return;
        }

        Mod::get()->setSavedValue("goal-level", levelId);
        Mod::get()->setSavedValue("goal-percent", goal);
        Mod::get()->setSavedValue("goal-attempts", 0);

        Notification::create("Goal saved.", NotificationIcon::Success, 2.f)->show();
        this->onClose(nullptr);
    }

public:
    static GoalPopup* create() {
        auto ret = new GoalPopup();
        if (ret->init(240.f, 160.f)) {
            ret->autorelease();
            return ret;
        }
        delete ret;
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
        auto levelId = Mod::get()->getSavedValue<std::string>("goal-level", "");
        int goal = Mod::get()->getSavedValue<int>("goal-percent", 0);
        int attempts = Mod::get()->getSavedValue<int>("goal-attempts", 0);

        if (levelId.empty() || goal <= 0) return;

        if (!m_fields->m_goalLabel) {
            m_fields->m_goalLabel = CCLabelBMFont::create("", "bigFont.fnt");
            m_fields->m_goalLabel->setAnchorPoint({0.5f, 1.f});
            m_fields->m_goalLabel->setScale(0.4f);
            m_fields->m_goalLabel->setZOrder(999);
            m_fields->m_goalLabel->setID("session-goals-label");

            auto winSize = CCDirector::get()->getWinSize();
            m_fields->m_goalLabel->setPosition({winSize.width / 2.f, winSize.height - 10.f});

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

        auto button = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Goals"),
            this,
            menu_selector(GoalsPauseLayer::onGoals)
        );

        button->setPosition({winSize.width / 2.f, winSize.height - 30.f});

        auto menu = CCMenu::create();
        menu->addChild(button);
        menu->setPosition({0.f, 0.f});
        menu->setZOrder(999);

        this->addChild(menu);
    }

    void onGoals(CCObject*) {
        GoalPopup::create()->show();
    }
};
