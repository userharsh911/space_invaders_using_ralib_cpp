#include <raylib.h>
#include <vector>
#include <iostream>
using namespace std;

class Game{
    protected:
    int screen_width;
    int screen_height;
    Color color;
    public:
    Game() : screen_width(1080), screen_height(720), color(BLACK){}
    void initialize_window(){
        InitWindow(screen_width, screen_height, "Space Invader");
    }
    void bg(){
        ClearBackground(color);
    }
};


class Bullet{
    protected:
    float width;
    float height;
    float speed;
    Texture2D texture;
    public:
    Vector2 bullet_pos;
    Bullet(){}
    Bullet(Vector2 pos, int speed, string filename) : width(5), height(20){
        bullet_pos = pos;
        this->speed = speed;
        Image image = LoadImage(&filename[0]);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    void draw(){
        // DrawRectangle
        DrawTexture(texture, bullet_pos.x, bullet_pos.y, WHITE);
    }
    void movement(){
        bullet_pos.y -= speed;
    }
    Vector2 get_dimensions(){
        Vector2 v = {width, height};
        return v;
    }
    ~Bullet(){UnloadTexture(texture);}
    
};

class Player : public Game, public Bullet{
    protected:
    Vector2 pos;
    int width;
    int height;
    float speed;
    Texture2D texture;
    public:
    vector<Bullet*> bullets;
    Player() : width(50), height(50), speed(10){
        pos.x = (screen_width / 2) - (this->width / 2);
        pos.y = screen_height - this->height;
        Image image = LoadImage("./src/images/player.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }  
    void draw(){
        // DrawRectangle(pos.x, pos.y, width, height,RED);
        DrawTexture(texture, pos.x, pos.y, WHITE);

    }
    void movement(){
        if(IsKeyDown(KEY_D)){
            pos.x += speed;
        }else if(IsKeyDown(KEY_A)){
            pos.x -= speed;
        }
    }
    void shoot(){
        if(IsKeyPressed(KEY_SPACE)){
            Bullet* bullet =  new Bullet({pos.x + (width / 2 - 5 / 2.0f), pos.y}, 10, "./src/images/player_bullet.png");
            bullets.push_back(bullet);
        }
    }
    void delete_bullets(){
        for(float i = 0; i < bullets.size(); i++){
            if(bullets[i] && bullets[i]->bullet_pos.y <= -20){
                delete bullets[i];
                bullets[i] = nullptr;
                bullets.erase(bullets.begin() + i);
            }
        }
    }
    bool collision(vector<Bullet*> &bullets){
        for(auto& bullet : bullets){
            if(bullet && bullet->bullet_pos.y + bullet->get_dimensions().y >= this->pos.y && bullet->bullet_pos.x > this->pos.x - bullet->get_dimensions().x && bullet->bullet_pos.x + bullet->get_dimensions().x < this->pos.x + this->width){
                delete bullet;
                bullet = nullptr;
                cout<<"Player hit"<<endl;
                return true;
            }
        }
        return false;
    }
    ~Player(){UnloadTexture(texture);}
};

class Enemies : public Game , public Bullet{
    static float speed;
    Texture2D texture;
    public:
    int lives;
    float width;
    float height;
    float padding;
    static vector<Bullet*> bullets;
    static int count;
    Vector2 pos;
    Enemies(float x, float y, float lives) : width(50), height(50), padding(30){
        pos.x = x;
        pos.y = y;
        this->lives = lives;
        Image image = LoadImage("./src/images/enemy.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }
    void set_coordinates(Vector2 pos){this->pos = pos;}
    void draw(){
        // DrawRectangle(pos.x, pos.y, width, height,GREEN);
        DrawTexture(texture, pos.x, pos.y, WHITE);

    }
    void movement(){pos.x += speed;}
    void bounce(){
        if(pos.x >= screen_width - width){
            speed = -abs(speed);
        }else if(pos.x <= 0){
            speed = abs(speed);
        }
    }
    void shoot(){
        Bullet* bullet = new Bullet({pos.x + (width / 2 - 5 / 2.0f), pos.y}, -10, "./src/images/enemy_bullet.png");
        bullets.push_back(bullet);
        cout<<"shoot"<<endl;
    }
    bool collision(Player & player){
        for(auto& bullet : player.bullets){
            if(bullet && bullet->bullet_pos.y <= pos.y + height && bullet->bullet_pos.y >= pos.y && bullet->bullet_pos.x > pos.x - bullet->get_dimensions().x && bullet->bullet_pos.x + bullet->get_dimensions().x < pos.x + width){
                delete bullet;
                bullet = nullptr;
                if(!this->lives) return true;
                this->lives--;
            }
        }
        return false;
    }
    static void delete_bullets(){
        for(float i = 0; i < bullets.size(); i++){
            if(bullets[i] && bullets[i]->bullet_pos.y + bullets[i]->get_dimensions().y > 720){
                delete bullets[i];
                bullets[i] = nullptr;
                bullets.erase(bullets.begin() + i);
            }
        }
    }
    ~Enemies(){count--;UnloadTexture(texture);}
    
};

float Enemies::speed = 3;
int Enemies::count = 0;
vector<Bullet*> Enemies::bullets = {};
int main(){
    Game game;
    game.initialize_window();
    Player player;
    vector<Enemies*> enemies;
    int col = 6;
    int row = 3;
    Enemies::count = col * row;
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            enemies.push_back(new Enemies(j * 50 + j * 50 , i * 50 + i * 50, row - i));
        }
    }


    SetTargetFPS(60);
    int c = -1;
    int space_count = 0;
    int space_times = 3;
    while(!WindowShouldClose()){
        player.shoot();
        space_count %= space_times;
        if(space_count == space_times - 1)c = GetRandomValue(0, Enemies::count - 1), space_count++;
        for(unsigned int i = 0; i < enemies.size(); i++){
            if(enemies[i]){
                if(!c)enemies[i]->shoot();
                c--;
                if(c < 0) break;
            }
        }
        if(IsKeyPressed(KEY_SPACE)) space_count++;
        for(auto &bullets : player.bullets) if(bullets) bullets->movement();
        for(auto &bullets : Enemies::bullets) if(bullets) bullets->movement();
        player.movement();
        for(auto& enemy : enemies) if(enemy) enemy->movement();
        for(auto& enemy : enemies) if(enemy) enemy->bounce();
        for(auto& enemy : enemies){
            if (enemy && enemy->collision(player)) delete enemy, enemy = nullptr;
        }
        if(Enemies::bullets.size() > 0) player.collision(Enemies::bullets);
        player.delete_bullets();
         if(Enemies::bullets.size() > 0) Enemies::delete_bullets();
        
        BeginDrawing();
        game.bg();
        player.draw();
        for(auto &bullets : player.bullets) if(bullets) bullets->draw();
        for(auto &bullets : Enemies::bullets) if(bullets) bullets->draw();
        for(auto& enemy : enemies) if(enemy) enemy->draw();
        EndDrawing();
    }
    CloseWindow();
}