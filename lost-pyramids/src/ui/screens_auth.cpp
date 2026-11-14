// Log in and Sign up screens.

#include "ui/screens.h"

#include "storage/account_storage.h"
#include "ui/location_theme.h"
#include "ui/theme.h"

namespace
{

void showStatus(App& app, const std::string& message, bool isError)
{
    app.statusMessage = message;
    app.statusIsError = isError;
}

void drawStatusMessage(const App& app, float y)
{
    if (app.statusMessage.empty())
    {
        return;
    }
    const Color color = app.statusIsError ? theme::DANGER : theme::SUCCESS;
    const float width = measureString(app.statusMessage.c_str(), 24.0f, FontStyle::Bold) + 40.0f;
    DrawRectangleRounded({SCREEN_CENTER_X - width / 2.0f, y - 6.0f, width, 38.0f}, 0.5f, 6, Fade(theme::PANEL, 0.9f));
    drawStringCentered(app.statusMessage.c_str(), SCREEN_CENTER_X, y, 24.0f, color, FontStyle::Bold);
}

void attemptLogin(App& app)
{
    const AuthResponse response = loginUser(USERS_FILE_PATH, app.loginUsername.text, app.loginPassword.text);
    app.loginPassword.text.clear();

    if (!response.success)
    {
        showStatus(app, response.message, true);
        return;
    }

    app.currentUser = response.username;
    app.statusMessage.clear();
    changeScreen(app, Screen::MainMenu);
}

void attemptSignUp(App& app)
{
    const AuthResponse response = registerUser(USERS_FILE_PATH, app.signUpUsername.text, app.signUpPassword.text,
                                               app.signUpConfirm.text, app.rng);
    app.signUpPassword.text.clear();
    app.signUpConfirm.text.clear();

    if (!response.success)
    {
        showStatus(app, response.message, true);
        return;
    }

    // Go back to the login screen with the new name already filled in.
    app.loginUsername.text = response.username;
    app.loginPassword.text.clear();
    app.signUpUsername.text.clear();
    showStatus(app, response.message, false);
    changeScreen(app, Screen::Login);
}
}  // namespace

void runLoginScreen(App& app)
{
    drawMenuBackdrop(static_cast<float>(GetTime()));
    drawGameTitle(40.0f, "Expeditions into the world's forgotten labyrinths");

    const Rectangle panel = {SCREEN_CENTER_X - 270.0f, 210.0f, 540.0f, 440.0f};
    drawPanel(panel, theme::GOLD_BRIGHT);
    drawStringCentered("ENTER THE EXPEDITION", SCREEN_CENTER_X, panel.y + 28.0f, 30.0f, theme::GOLD_BRIGHT, FontStyle::Title);

    const Rectangle userBox = {panel.x + 50.0f, panel.y + 120.0f, panel.width - 100.0f, 50.0f};
    const Rectangle passBox = {panel.x + 50.0f, panel.y + 212.0f, panel.width - 100.0f, 50.0f};
    TextField* fields[] = {&app.loginUsername, &app.loginPassword};
    const Rectangle boxes[] = {userBox, passBox};
    updateTextFields(fields, boxes, 2);
    drawTextField(app.loginUsername, userBox);
    drawTextField(app.loginPassword, passBox);

    bool submit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    const float halfWidth = (panel.width - 110.0f) / 2.0f;

    if (drawButton({panel.x + 50.0f, panel.y + 290.0f, panel.width - 100.0f, 54.0f}, "Log In", false))
    {
        submit = true;
    }
    if (drawButton({panel.x + 50.0f, panel.y + 362.0f, halfWidth, 48.0f}, "Sign Up", false))
    {
        app.statusMessage.clear();
        changeScreen(app, Screen::SignUp);
        return;
    }
    if (drawButton({panel.x + 60.0f + halfWidth, panel.y + 362.0f, halfWidth, 48.0f}, "Quit", false))
    {
        app.screen = Screen::Exit;
        return;
    }

    drawStatusMessage(app, panel.y + panel.height + 20.0f);
    drawStringCentered("Tab - next field      Enter - log in", SCREEN_CENTER_X, static_cast<float>(SCREEN_HEIGHT) - 40.0f, 20.0f,
                       theme::PARCHMENT, FontStyle::Body);

    if (submit)
    {
        attemptLogin(app);
    }
}

void runSignUpScreen(App& app)
{
    drawMenuBackdrop(static_cast<float>(GetTime()));
    drawGameTitle(24.0f, "Join the expedition society");

    const Rectangle panel = {SCREEN_CENTER_X - 290.0f, 172.0f, 580.0f, 555.0f};
    drawPanel(panel, theme::GOLD_BRIGHT);
    drawStringCentered("CREATE AN ACCOUNT", SCREEN_CENTER_X, panel.y + 26.0f, 30.0f, theme::GOLD_BRIGHT, FontStyle::Title);

    const float boxX = panel.x + 50.0f;
    const float boxWidth = panel.width - 100.0f;
    const Rectangle userBox = {boxX, panel.y + 112.0f, boxWidth, 48.0f};
    const Rectangle passBox = {boxX, panel.y + 198.0f, boxWidth, 48.0f};
    const Rectangle confirmBox = {boxX, panel.y + 284.0f, boxWidth, 48.0f};

    TextField* fields[] = {&app.signUpUsername, &app.signUpPassword, &app.signUpConfirm};
    const Rectangle boxes[] = {userBox, passBox, confirmBox};
    updateTextFields(fields, boxes, 3);
    drawTextField(app.signUpUsername, userBox);
    drawTextField(app.signUpPassword, passBox);
    drawTextField(app.signUpConfirm, confirmBox);

    drawString("Username: 3-16 letters, digits or _, starting with a letter", boxX, panel.y + 344.0f, 19.0f,
               theme::TEXT_MUTED, FontStyle::Body);
    drawString("Password: 6-24 characters with a letter and a digit", boxX, panel.y + 368.0f, 19.0f, theme::TEXT_MUTED,
               FontStyle::Body);

    bool submit = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_KP_ENTER);
    if (drawButton({boxX, panel.y + 408.0f, boxWidth, 54.0f}, "Create Account", false))
    {
        submit = true;
    }
    if (drawButton({boxX, panel.y + 478.0f, boxWidth, 48.0f}, "Back to Log In", false) || IsKeyPressed(KEY_ESCAPE))
    {
        app.statusMessage.clear();
        changeScreen(app, Screen::Login);
        return;
    }

    drawStatusMessage(app, panel.y + panel.height + 18.0f);

    if (submit)
    {
        attemptSignUp(app);
    }
}
