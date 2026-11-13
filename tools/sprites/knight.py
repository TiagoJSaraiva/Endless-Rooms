"""Gerador do sprite do cavaleiro (Endless Rooms).

Como rodar (a partir da raiz do repositório; requer Python 3 com Pillow):
    python tools/sprites/knight.py
Opcional: --preview <pasta> grava versões ampliadas ×4 (não versionadas).

Gera, de forma determinística, em game/assets/img/character/cavaleiro/:
  idle_sprite.png          76x136
  walking_spritesheet.png  76x544 (4 quadros de 76x136 empilhados na vertical)
O desenho é feito na grade base de 38x68 e ampliado por SPRITE_SCALE (vizinho mais próximo,
preserva o pixel art). O tamanho final é o de CreateKnightCharacter (player.cpp).
O personagem é desenhado de frente, com os pés na base do quadro.
"""
import os
import sys
from PIL import Image

FRAME_W, FRAME_H = 38, 68
# Ampliação aplicada só na saída: o personagem fica maior na tela sem redesenhar a grade.
SPRITE_SCALE = 2
WALK_FRAMES = 4
OUT_DIR = os.path.join("game", "assets", "img", "character", "cavaleiro")

# Cada material tem (luz, base, sombra). O contorno é escuro e comum a todos.
OUTLINE = (18, 16, 24, 255)
STEEL = ((190, 198, 214, 255), (140, 150, 172, 255), (88, 96, 120, 255))
DARK_STEEL = ((120, 128, 150, 255), (84, 90, 112, 255), (54, 58, 78, 255))
RED = ((214, 70, 64, 255), (172, 38, 44, 255), (110, 24, 36, 255))   # cor de destaque
LEATHER = ((130, 90, 60, 255), (96, 64, 44, 255), (62, 40, 32, 255))
GOLD = ((255, 224, 120, 255), (222, 170, 60, 255), (160, 110, 40, 255))
VISOR = (10, 8, 14, 255)
EYE = (255, 236, 150, 255)

# Partes: (material, x0, y0, x1, y1) com limites inclusivos, no quadro em repouso.
# Ordem = ordem de desenho (de trás para a frente).
CX = 19  # coluna central
HEAD = [
    ("plume", 17, 0, 21, 5),
    ("helmet", 12, 4, 25, 19),
]
TORSO = [
    ("steel", 11, 21, 26, 38),
    ("belt", 11, 38, 26, 40),
    ("tabard", 15, 40, 22, 52),
]
ARM_L = ("steel", 5, 23, 10, 36)    # braço esquerdo da tela
ARM_R = ("steel", 27, 23, 32, 36)
SHOULDER_L = ("dark", 4, 21, 11, 27)
SHOULDER_R = ("dark", 26, 21, 33, 27)
HAND_L = ("leather", 5, 36, 10, 41)
HAND_R = ("leather", 27, 36, 32, 41)
LEG_L = ("dark", 12, 41, 18, 61)
LEG_R = ("dark", 20, 41, 26, 61)
BOOT_L = ("leather", 11, 61, 18, 67)
BOOT_R = ("leather", 20, 61, 27, 67)

MATERIALS = {
    "steel": STEEL, "dark": DARK_STEEL, "leather": LEATHER,
    "helmet": STEEL, "plume": RED, "tabard": RED, "belt": GOLD,
}

# Deslocamentos por quadro: (bob do corpo, dy perna esq, dy perna dir, dy braço esq, dy braço dir)
# dy negativo = levanta. Braços balançam em oposição às pernas.
WALK_POSES = [
    (0, -3, 0, 2, -2),
    (-1, 0, 0, 0, 0),
    (0, 0, -3, -2, 2),
    (-1, 0, 0, 0, 0),
]
IDLE_POSE = (0, 0, 0, 0, 0)


def put(img, x, y, color):
    if 0 <= x < FRAME_W and 0 <= y < FRAME_H:
        img.putpixel((x, y), color)


def box(img, mat, x0, y0, x1, y1, round_top=False):
    """Retângulo com contorno, luz à esquerda e sombra à direita."""
    light, base, shade = MATERIALS.get(mat, STEEL)
    for y in range(y0, y1 + 1):
        for x in range(x0, x1 + 1):
            corner = (x in (x0, x1)) and (y in (y0, y1))
            if corner and (round_top or y == y1):
                continue
            edge = x in (x0, x1) or y in (y0, y1)
            if edge:
                c = OUTLINE
            elif x == x0 + 1 or y == y0 + 1:
                c = light
            elif x >= x1 - 2 or y == y1 - 1:
                c = shade
            else:
                c = base
            put(img, x, y, c)


def draw_frame(pose):
    bob, legl, legr, arml, armr = pose
    img = Image.new("RGBA", (FRAME_W, FRAME_H), (0, 0, 0, 0))

    def moved(part, dy):
        m, x0, y0, x1, y1 = part
        return (m, x0, y0 + dy, x1, y1 + dy)

    # Pernas e botas: perna levantada encurta (a bota sobe, a base não muda de largura).
    for leg, boot, dy in ((LEG_L, BOOT_L, legl), (LEG_R, BOOT_R, legr)):
        m, x0, y0, x1, y1 = leg
        box(img, m, x0, y0, x1, y1 + dy)
        bm, bx0, by0, bx1, by1 = boot
        box(img, bm, bx0, by0 + dy, bx1, by1 + dy)

    # Tudo acima da cintura desce/sobe com o bob.
    for part in TORSO:
        m, x0, y0, x1, y1 = moved(part, bob)
        box(img, m, x0, y0, x1, y1)
    # Detalhe do tabardo: faixa dourada central.
    for y in range(TORSO[2][2] + bob + 1, TORSO[2][4] + bob):
        put(img, CX, y, GOLD[1])
        put(img, CX + 1, y, GOLD[2])
    # Peitoral: linha central de realce.
    for y in range(TORSO[0][2] + bob + 3, TORSO[0][4] + bob - 1):
        put(img, CX, y, STEEL[0])

    for arm, hand, d in ((ARM_L, HAND_L, arml), (ARM_R, HAND_R, armr)):
        m, x0, y0, x1, y1 = moved(arm, bob + d)
        box(img, m, x0, y0, x1, y1)
        hm, hx0, hy0, hx1, hy1 = moved(hand, bob + d)
        box(img, hm, hx0, hy0, hx1, hy1, round_top=True)
    for sh in (SHOULDER_L, SHOULDER_R):
        m, x0, y0, x1, y1 = moved(sh, bob)
        box(img, m, x0, y0, x1, y1, round_top=True)

    # Cabeça: pluma atrás do elmo.
    for m, x0, y0, x1, y1 in HEAD:
        box(img, m, x0, y0 + bob, x1, y1 + bob, round_top=True)
    hy = bob
    # Viseira com fenda escura e brilho dos olhos.
    for x in range(14, 24):
        put(img, x, 10 + hy, VISOR)
        put(img, x, 11 + hy, VISOR)
    put(img, 16, 11 + hy, EYE)
    put(img, 21, 11 + hy, EYE)
    # Respiros do elmo (queixo).
    for y in (14, 16):
        for x in (17, 19, 21):
            put(img, x, y + hy, STEEL[2])
    # Crista central do elmo.
    for y in range(6, 10):
        put(img, CX, y + hy, STEEL[0])
    return img


def upscale(img):
    return img.resize((img.width * SPRITE_SCALE, img.height * SPRITE_SCALE), Image.NEAREST)


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    idle = draw_frame(IDLE_POSE)
    sheet = Image.new("RGBA", (FRAME_W, FRAME_H * WALK_FRAMES), (0, 0, 0, 0))
    for i, pose in enumerate(WALK_POSES):
        sheet.paste(draw_frame(pose), (0, i * FRAME_H))
    upscale(idle).save(os.path.join(OUT_DIR, "idle_sprite.png"))
    upscale(sheet).save(os.path.join(OUT_DIR, "walking_spritesheet.png"))
    if "--preview" in sys.argv:
        folder = sys.argv[sys.argv.index("--preview") + 1]
        os.makedirs(folder, exist_ok=True)
        # Quadros lado a lado sobre fundo escuro, ampliados ×4.
        strip = Image.new("RGBA", (FRAME_W * (WALK_FRAMES + 1), FRAME_H), (34, 30, 40, 255))
        strip.alpha_composite(idle, (0, 0))
        for i in range(WALK_FRAMES):
            strip.alpha_composite(sheet.crop((0, i * FRAME_H, FRAME_W, (i + 1) * FRAME_H)), (FRAME_W * (i + 1), 0))
        strip.resize((strip.width * 4, strip.height * 4), Image.NEAREST).save(os.path.join(folder, "knight_x4.png"))


if __name__ == "__main__":
    main()
