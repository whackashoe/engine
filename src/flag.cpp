#include "flag.hpp"

void flag::add_to_world(b2World * world)
{
    const settings& config = settings::get_instance();

    b2BodyDef bdef;
    bdef.type = b2_staticBody;
    bdef.position.Set(this->x, this->y);

    body = world->CreateBody(&bdef);

    b2CircleShape bshape;
    bshape.m_p.Set(0.0f, 0.0f);
    bshape.m_radius = config.FLAG_RADIUS;

    b2FixtureDef fdef;
    fdef.shape = &bshape;
    fdef.isSensor = true;
    body->CreateFixture(&fdef);
    body->SetUserData(static_cast<void*>(col_data));
}

void flag::step_on(ball* m)
{
    std::cout << "flag stepped on" << std::endl;
}

void to_json(nlohmann::json& j, const flag& p)
{
    j = nlohmann::json{{"x", p.x}, {"y", p.y}, {"type", to_string(p.type)}};
}

void from_json(const nlohmann::json& j, flag& p)
{
    p.x    = j.at("x").get<float>();
    p.y    = j.at("y").get<float>();
    p.type = flag_type_from_string(j.at("type").get<std::string>());
}
