from veriloggen import *

from src.util.per_enum import ArchType
from src.util.per_graph import PeRGraph
from src.util.piplinebase import PiplineBase
from src.util.util import Util
from src.util.hw_components import HwComponents


class YotoPipelineHw(PiplineBase):
    def __init__(self, per_graph: PeRGraph, arch_type: ArchType, distance_table_bits: int, make_shuffle: bool,
                 n_threads: int = 1):
        self.len_pipeline: int = 6
        super().__init__(per_graph, arch_type, distance_table_bits, make_shuffle, self.len_pipeline, n_threads, )
        self.hw_components = HwComponents()
        self.th_bits = Util.get_n_bits(self.n_threads)
        self.edge_bits = Util.get_n_bits(self.per_graph.n_cells)
        self.node_bits = Util.get_n_bits(self.per_graph.n_cells)
        self.ij_bits = Util.get_n_bits(self.n_lines)

    # todo under construction
    def create_rom_files(self, edges_rom_f: str, n2c_rom_f: str, dst_tbl_rom_f: str,
                         cell_content_f: str, n2c: list[list[list]]):
        # Function to create ROM content
        def generate_rom_content(addr_bits: int, file_bits: int, data: list[list]):
            file_content = ['{0:b}'.format(0).zfill(file_bits) for _ in range(pow(2, addr_bits))]
            for i, entries in enumerate(data):
                for entry_idx, entry in enumerate(entries):
                    if entry[0] is None:
                        continue
                    idx = (i << addr_bits) | entry_idx
                    idx_str = '{0:b}'.format(idx).zfill(addr_bits)
                    content = (entry[0] << file_bits // 2) | entry[1]
                    file_content[idx] = '{0:b}'.format(content).zfill(file_bits)
            return file_content

        # edges rom file
        edges_file_bits = self.node_bits * 2
        edges_addr_bits = self.th_bits + self.edge_bits
        edges_file_content = generate_rom_content(edges_addr_bits, edges_file_bits, self.edges_int)

        # n2c rom file
        n2c_file_bits = 2 * self.ij_bits
        n2c_addr_bits = self.th_bits + self.node_bits
        n2c_file_content = generate_rom_content(n2c_addr_bits, n2c_file_bits, n2c)

        '''
        # dst_table rom file
        dts_table_file_bits = 2 * (self.ij_bits + 1)
        dts_table_addr_bits = self.distance_table_bits + 5
        dts_table_file_content = ['{0:b}'.format(0).zfill(dts_table_file_bits) for _ in
                                  range(pow(2, dts_table_addr_bits))]
        dst_table: list[list[list]] = [
            Util.get_distance_table(self.arch_type, self.n_lines, self.make_shuffle) for _ in
            range(self.distance_table_bits)]
        for line, n2cs in enumerate(n2c):
            for node_idx, n2c_ in enumerate(n2cs):
                if n2c_[0] is None:
                    continue
                idx: int = th << self.node_bits | node_idx
                idx_s: str = '{0:b}'.format(idx).zfill(n2c_addr_bits)
                n2c_content = n2c_[0] << self.ij_bits | n2c_[1]
                n2c_file_content[idx] = '{0:b}'.format(n2c_content).zfill(n2c_file_bits)
                break'''

    def create_yoto_pipeline_hw_test_bench(self, v_output_base: str, simulate: bool):
        edges_rom_f: str = f'{v_output_base}%s_edges.rom' % self.per_graph.dot_name
        n2c_rom_f: str = f'{v_output_base}%s_n2c.rom' % self.per_graph.dot_name
        n2c_out_f: str = f'{v_output_base}%s_n2c_out.txt' % self.per_graph.dot_name
        dst_tbl_rom_f: str = f'{v_output_base}%s_dst_tbl.rom' % self.per_graph.dot_name
        cell_content_f: str = f'{v_output_base}%s_cell_content.rom' % self.per_graph.dot_name

        first_nodes: list = [self.edges_int[i][0][0] for i in range(self.len_pipeline)]
        n2c, c2n = self.init_traversal_placement_tables(first_nodes)

        self.create_rom_files(edges_rom_f, n2c_rom_f, dst_tbl_rom_f, cell_content_f, n2c)

        name = '%s_yoto_pip_hw_test_bench' % self.per_graph.dot_name
        m = Module(name)

        clk = m.Reg('clk')
        rst = m.Reg('rst')
        start = m.Reg('start')

        yoto_visited_edges = m.Wire('yoto_visited_edges', self.edge_bits)
        yoto_done = m.Wire('yoto_done')
        yoto_total_pipeline_counter = m.Wire('yoto_total_pipeline_counter', 32)

        par = []
        con = [
            ('clk', clk),
            ('rst', rst),
            ('start', start),
            ('visited_edges', yoto_visited_edges),
            ('done', yoto_done),
            ('total_pipeline_counter', yoto_total_pipeline_counter),
        ]
        yoto = self.create_yoto_pipeline_hw(edges_rom_f, n2c_rom_f, n2c_out_f, dst_tbl_rom_f, cell_content_f, simulate)
        m.Instance(yoto, yoto.name, par, con)

        verilog_f: str = f'{v_output_base}{m.name}.v'
        m.to_verilog(verilog_f)

    def create_yoto_pipeline_hw(self, edges_rom_f: str, n2c_rom_f: str, n2c_out_f: str, dst_tbl_rom_f: str,
                                cell_content_f: str, simulate: bool) -> Module:
        name = "yoto_pipeline_hw"
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')
        start = m.Input('start')
        visited_edges = m.Input('visited_edges', self.edge_bits)
        done = m.Output('done')
        total_pipeline_counter = m.Output('total_pipeline_counter', 32)

        m.EmbeddedCode('// St0 wires')
        st0_th_idx = m.Wire('st0_th_idx', self.th_bits)
        st0_th_valid = m.Wire('st0_th_valid')
        st0_edg_n = m.Wire('st0_edg_n', self.edge_bits)
        st0_incr_edg = m.Wire('st0_incr_edg')
        m.EmbeddedCode('// -----')
        m.EmbeddedCode('')

        m.EmbeddedCode('// St1 wires')
        st1_th_idx = m.Wire('st1_th_idx', self.th_bits)
        st1_th_valid = m.Wire('st1_th_valid')
        st1_dist_table_line = m.Wire('st1_dist_table_line', self.distance_table_bits)
        st1_a = m.Wire('st1_a', self.node_bits)
        st1_b = m.Wire('st1_b', self.node_bits)
        m.EmbeddedCode('// -----')
        m.EmbeddedCode('')

        m.EmbeddedCode('// St2 wires')
        st2_th_idx = m.Wire('st2_th_idx', self.th_bits)
        st2_th_valid = m.Wire('st2_th_valid')
        st2_ia = m.Wire('st2_ia', self.ij_bits)
        st2_ja = m.Wire('st2_ja', self.ij_bits)
        st2_dist_table_line = m.Wire('st2_dist_table_line', self.distance_table_bits)
        st2_dist_counter = m.Wire('st2_dist_counter', 6)
        st2_b = m.Wire('st2_b', self.node_bits)
        m.EmbeddedCode('// -----')
        m.EmbeddedCode('')

        m.EmbeddedCode('// St3 wires')
        st3_th_idx = m.Wire('st3_th_idx', self.th_bits)
        st3_th_valid = m.Wire('st3_th_valid')
        st3_ib = m.Wire('st3_ib', self.ij_bits + 1)
        st3_jb = m.Wire('st3_jb', self.ij_bits + 1)
        st3_dist_counter = m.Wire('st3_dist_counter', 6)
        st3_b = m.Wire('st3_b', self.node_bits)
        m.EmbeddedCode('// -----')
        m.EmbeddedCode('')

        m.EmbeddedCode('// St4 wires')
        st4_th_idx = m.Wire('st4_th_idx', self.th_bits)
        st4_th_valid = m.Wire('st4_th_valid')
        st4_place = m.Wire('st4_place')
        st4_dist_counter = m.Wire('st4_dist_counter', 6)
        st4_ib = m.Wire('st4_ib', self.ij_bits)
        st4_jb = m.Wire('st4_jb', self.ij_bits)
        st4_b = m.Wire('st4_b', self.node_bits)
        m.EmbeddedCode('// -----')
        m.EmbeddedCode('')

        m.EmbeddedCode('// St0 instantiation')
        stage0_m = self.create_stage0_yoto()
        par = []
        con = [
            ('clk', clk),
            ('rst', rst),
            ('start', start),
            ('done', done),
            ('visited_edges', visited_edges),
            ('th_idx', st0_th_idx),
            ('th_valid', st0_th_valid),
            ('edg_n', st0_edg_n),
            ('incr_edg', st0_incr_edg),
            ('total_pipeline_counter', total_pipeline_counter),
            ('st4_place', st4_place),
        ]
        m.Instance(stage0_m, stage0_m.name, par, con)
        m.EmbeddedCode('// -----')

        m.EmbeddedCode('// St1 instantiation')
        stage1_m = self.create_stage1_yoto(edges_rom_f, simulate)
        con = [
            ('clk', clk),
            ('rst', rst),
            ('th_idx', st1_th_idx),
            ('th_valid', st1_th_valid),
            ('dist_table_line', st1_dist_table_line),
            ('a', st1_a),
            ('b', st1_b),
            ('st0_th_idx', st0_th_idx),
            ('st0_th_valid', st0_th_valid),
            ('st0_edg_n', st0_edg_n),
        ]
        m.Instance(stage1_m, stage1_m.name, par, con)
        m.EmbeddedCode('// -----')

        m.EmbeddedCode('// St2 instantiation')
        stage2_m = self.create_stage2_yoto(n2c_rom_f, n2c_out_f, simulate)
        con = [
            ('clk', clk),
            ('rst', rst),
            ('th_idx', st2_th_idx),
            ('th_valid', st2_th_valid),
            ('ia', st2_ia),
            ('ja', st2_ja),
            ('dist_table_line', st2_dist_table_line),
            ('dist_counter', st2_dist_counter),
            ('b', st2_b),
            ('st1_th_idx', st1_th_idx),
            ('st1_th_valid', st1_th_valid),
            ('st1_dist_table_line', st1_dist_table_line),
            ('st1_a', st1_a),
            ('st1_b', st1_b),
            ('st4_th_idx', st4_th_idx),
            ('st4_th_valid', st4_th_valid),
            ('st4_place', st4_place),
            ('st4_dist_counter', st4_dist_counter),
            ('st4_ib', st4_ib),
            ('st4_jb', st4_jb),
            ('st4_b', st4_b),
        ]
        m.Instance(stage2_m, stage2_m.name, par, con)
        m.EmbeddedCode('// -----')

        m.EmbeddedCode('// St3 instantiation')
        stage3_m = self.create_stage3_yoto(dst_tbl_rom_f, simulate)
        con = [
            ('clk', clk),
            ('rst', rst),
            ('th_idx', st3_th_idx),
            ('th_valid', st3_th_valid),
            ('ib', st3_ib),
            ('jb', st3_jb),
            ('dist_counter', st3_dist_counter),
            ('b', st3_b),
            ('st2_th_idx', st2_th_idx),
            ('st2_th_valid', st2_th_valid),
            ('st2_ia', st2_ia),
            ('st2_ja', st2_ja),
            ('st2_dist_table_line', st2_dist_table_line),
            ('st2_dist_counter', st2_dist_counter),
            ('st2_b', st2_b),
        ]
        m.Instance(stage3_m, stage3_m.name, par, con)
        m.EmbeddedCode('// -----')

        m.EmbeddedCode('// St4 instantiation')
        stage4_m = self.create_stage4_yoto(cell_content_f, simulate)
        con = [
            ('clk', clk),
            ('rst', rst),
            ('th_idx', st4_th_idx),
            ('th_valid', st4_th_valid),
            ('place', st4_place),
            ('ib', st4_ib),
            ('jb', st4_jb),
            ('dist_counter', st4_dist_counter),
            ('b', st4_b),
            ('st3_th_idx', st3_th_idx),
            ('st3_th_valid', st3_th_valid),
            ('st3_ib', st3_ib),
            ('st3_jb', st3_jb),
            ('st3_dist_counter', st3_dist_counter),
            ('st3_b', st3_b),
        ]
        m.Instance(stage4_m, stage4_m.name, par, con)
        m.EmbeddedCode('// -----')

        return m

    def create_stage0_yoto(self) -> Module:
        name = 'stage0_yoto'
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')
        start = m.Input('start')
        done = m.OutputReg('done')

        visited_edges = m.Input('visited_edges', self.edge_bits)

        th_idx = m.OutputReg('th_idx', self.th_bits)
        th_valid = m.OutputReg('th_valid')
        edg_n = m.OutputReg('edg_n', self.edge_bits)
        incr_edg = m.OutputReg('incr_edg')
        total_pipeline_counter = m.OutputReg('total_pipeline_counter', 32)
        st4_place = m.Input('st4_place')

        st0_th_idx = m.Wire('st0_th_idx', self.th_bits)
        st0_th_valid = m.Wire('st0_th_valid')
        st0_edg_n = m.Wire('st0_edg_n', self.edge_bits)
        st0_incr_edge = m.Wire('st0_incr_edge')

        edge_counter = m.Reg('edge_counter', self.edge_bits + 1, self.n_threads)
        thread_valid = m.Reg('thread_valid', self.n_threads)
        thread_done = m.Reg('thread_done', self.n_threads)
        next_th_idx = m.Reg('next_th_idx', self.th_bits + 1)
        running = m.Reg('running')

        m.EmbeddedCode('')
        st0_th_idx.assign(th_idx)
        st0_th_valid.assign(th_valid)
        st0_edg_n.assign(edg_n)
        st0_incr_edge.assign(incr_edg)

        m.Always(Posedge(clk))(
            If(rst)(
                done(Int(0, 1, 10)),
                total_pipeline_counter(Int(0, total_pipeline_counter.width, 10)),

                thread_valid(Int(pow(2, self.n_threads) - 1, thread_valid.width, 2)),
                thread_done(Int(0, thread_done.width, 10)),
                next_th_idx(Int(0, next_th_idx.width, 10)),

                th_idx(Int(0, 1, 10)),
                th_valid(Int(0, 1, 10)),
                edg_n(Int(0, edg_n.width, 10)),
                incr_edg(Int(0, 1, 10)),

                running(Int(0, 1, 10)),
            ).Elif(start)(
                total_pipeline_counter(total_pipeline_counter + Int(1, total_pipeline_counter.width, 10)),

                th_idx(next_th_idx),
                th_valid(thread_valid[next_th_idx]),
                incr_edg(st4_place),

                If(next_th_idx == Int(self.len_pipeline - 1, next_th_idx.width, 10))(
                    next_th_idx(Int(0, next_th_idx.width, 10)),
                    running(Int(1, 1, 10)),
                ).Else(
                    next_th_idx(next_th_idx + Int(1, next_th_idx.width, 10)),
                ),
                If(st0_incr_edge)(
                    edge_counter[st0_th_idx](st0_edg_n),
                ),
                If(AndList(st0_th_valid, st0_incr_edge, st0_edg_n == visited_edges))(
                    thread_valid[st0_th_idx](Int(0, 1, 10)),
                    thread_done[st0_th_idx](Int(1, 1, 10)),
                ),
                If(Uand(thread_done))(
                    done(Int(1, 1, 10))
                ),
                If(~running)(
                    edg_n(Int(0, edg_n.width, 10)),
                ).Else(
                    edg_n(Mux(~st4_place,
                              edge_counter[th_idx],
                              edge_counter[th_idx] + Int(1, edg_n.width)
                              )),
                )
            )
        )
        return m

    def create_stage1_yoto(self, edges_rom_f: str, simulate: bool) -> Module:
        name = 'stage1_yoto'
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')

        th_idx = m.OutputReg('th_idx', self.th_bits)
        th_valid = m.OutputReg('th_valid')
        dist_table_line = m.OutputReg('dist_table_line', self.distance_table_bits)
        a = m.OutputReg('a', self.node_bits)
        b = m.OutputReg('b', self.node_bits)

        st0_th_idx = m.Input('st0_th_idx', self.th_bits)
        st0_th_valid = m.Input('st0_th_valid')
        st0_edg_n = m.Input('st0_edg_n', self.edge_bits)

        a_t = m.Wire('a_t', self.node_bits)
        b_t = m.Wire('b_t', self.node_bits)

        m.Always(Posedge(clk))(
            If(rst)(
                th_idx(Int(0, th_idx.width, 10)),
                th_valid(Int(0, th_valid.width, 10)),
                dist_table_line(Int(0, dist_table_line.width, 10)),
                a(Int(0, a.width, 10)),
                b(Int(0, b.width, 10)),
            ).Else(
                th_idx(st0_th_idx),
                th_valid(st0_th_valid),
                a(a_t),
                b(b_t),
                dist_table_line(Xor(Cat(Int(0, self.distance_table_bits - self.th_bits, 10), st0_th_idx),
                                    st0_edg_n[0:self.distance_table_bits])),
            )
        )

        par = [
            ('width', self.node_bits * 2),
            ('depth', self.th_bits + self.edge_bits),
        ]
        if simulate:
            par.append(('read_f', 1))
            par.append(('init_file', edges_rom_f), )

        con = [
            ('rd_addr', Cat(st0_th_idx, st0_edg_n)),
            ('out', Cat(a_t, b_t)),
            ('wr', Int(0, 1, 10)),
            ('wr_addr', Int(0, self.edge_bits + self.th_bits, 10)),
            ('wr_data', Int(0, self.node_bits * 2, 10)),
        ]

        edges_m = self.hw_components.create_memory_1r_1w()
        m.Instance(edges_m, edges_m.name, par, con)

        return m

    def create_stage2_yoto(self, n2c_rom_f: str, n2c_out_f: str, simulate: bool) -> Module:
        name = 'stage2_yoto'
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')

        th_idx = m.OutputReg('th_idx', self.th_bits)
        th_valid = m.OutputReg('th_valid')
        ia = m.OutputReg('ia', self.ij_bits)
        ja = m.OutputReg('ja', self.ij_bits)
        dist_table_line = m.OutputReg('dist_table_line', self.distance_table_bits)
        dist_counter = m.OutputReg('dist_counter', 6)
        b = m.OutputReg('b', self.node_bits)

        st1_th_idx = m.Input('st1_th_idx', self.th_bits)
        st1_th_valid = m.Input('st1_th_valid')
        st1_dist_table_line = m.Input('st1_dist_table_line', self.distance_table_bits)
        st1_a = m.Input('st1_a', self.node_bits)
        st1_b = m.Input('st1_b', self.node_bits)

        st4_th_idx = m.Input('st4_th_idx', self.th_bits)
        st4_th_valid = m.Input('st4_th_valid')
        st4_place = m.Input('st4_place')
        st4_dist_counter = m.Input('st4_dist_counter', 6)
        st4_ib = m.Input('st4_ib', self.ij_bits)
        st4_jb = m.Input('st4_jb', self.ij_bits)
        st4_b = m.Input('st4_b', self.node_bits)

        th_dist_table_counter = m.Reg('th_dist_table_counter', 6, self.n_threads)
        running = m.Reg('running')
        ia_t = m.Wire('ia_t', self.ij_bits)
        ja_t = m.Wire('ja_t', self.ij_bits)

        m.Always(Posedge(clk))(
            If(rst)(
                th_idx(Int(0, th_idx.width, 10)),
                th_valid(Int(0, 1, 10)),
                ia(Int(0, ia.width, 10)),
                ja(Int(0, ja.width, 10)),
                dist_table_line(Int(0, dist_table_line.width, 10)),
                dist_counter(Int(0, dist_counter.width, 10)),
                b(Int(0, b.width, 10)),
                running(Int(0, 1, 10))
            ).Else(
                th_idx(st1_th_idx),
                th_valid(st1_th_valid),
                ia(ia_t),
                ja(ja_t),
                dist_table_line(st1_dist_table_line),
                b(st1_b),
                If(st4_place)(
                    th_dist_table_counter[st4_th_idx](Int(0, th_dist_table_counter.width, 10))
                ).Elif(st4_th_valid)(
                    th_dist_table_counter[st4_th_idx](st4_dist_counter + Int(1, th_dist_table_counter.width, 10))
                ),
                If(~running)(
                    dist_counter(Int(0, dist_counter.width, 10))
                ).Else(
                    dist_counter(th_dist_table_counter[st1_th_idx])
                ),
                If(st1_th_idx == Int(self.len_pipeline - 1, st1_th_idx.width, 10))(
                    running(Int(1, 1, 10)),
                ),
            )
        )

        par = [
            ('width', self.ij_bits * 2),
            ('depth', self.th_bits + self.node_bits)
        ]
        if simulate:
            par.append(('read_f', 1))
            par.append(('init_file', n2c_rom_f))
            par.append(('write_f', 1))
            par.append(('output_file', n2c_out_f))

        con = [
            ('rd_addr', Cat(st1_th_idx, st1_a)),
            ('out', Cat(ia_t, ja_t)),
            ('wr', st4_place),
            ('wr_addr', Cat(st4_th_idx, st4_b)),
            ('wr_data', Cat(st4_ib, st4_jb)),
        ]

        n2c_m = self.hw_components.create_memory_1r_1w()
        m.Instance(n2c_m, n2c_m.name, par, con)

        return m

    def create_stage3_yoto(self, dst_tbl_rom_f: str, simulate: bool) -> Module:
        name = 'stage3_yoto'
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')

        th_idx = m.OutputReg('th_idx', self.th_bits)
        th_valid = m.OutputReg('th_valid')
        ib = m.OutputReg('ib', self.ij_bits + 1)
        jb = m.OutputReg('jb', self.ij_bits + 1)
        dist_counter = m.OutputReg('dist_counter', 6)
        b = m.OutputReg('b', self.node_bits)

        st2_th_idx = m.Input('st2_th_idx', self.th_bits)
        st2_th_valid = m.Input('st2_th_valid')
        st2_ia = m.Input('st2_ia', self.ij_bits)
        st2_ja = m.Input('st2_ja', self.ij_bits)
        st2_dist_table_line = m.Input('st2_dist_table_line', self.distance_table_bits)
        st2_dist_counter = m.Input('st2_dist_counter', 6)
        st2_b = m.Input('st2_b', self.node_bits)

        add_i_t = m.Wire('add_i_t', self.ij_bits + 1)
        add_j_t = m.Wire('add_j_t', self.ij_bits + 1)

        m.Always(Posedge(clk))(
            If(rst)(
                th_idx(Int(0, th_idx.width, 10)),
                th_valid(Int(0, 1, 10)),
                ib(Int(0, ib.width, 10)),
                jb(Int(0, jb.width, 10)),
                dist_counter(Int(0, dist_counter.width, 10)),
                b(Int(0, b.width, 10)),
            ).Else(
                th_idx(st2_th_idx),
                th_valid(st2_th_valid),
                dist_counter(st2_dist_counter),
                b(st2_b),
                ib(Cat(Int(0, 1, 10), st2_ia) + add_i_t),
                jb(Cat(Int(0, 1, 10), st2_ja) + add_j_t),
            )
        )

        par = [
            ('width', (self.ij_bits + 1) * 2),
            ('depth', 6 + self.distance_table_bits)
        ]
        if simulate:
            par.append(('read_f', 1))
            par.append(('init_file', dst_tbl_rom_f))
        con = [
            ('rd_addr', Cat(st2_dist_table_line, st2_dist_counter)),
            ('out', Cat(add_i_t, add_j_t)),
            ('wr', Int(0, 1, 10)),
            ('wr_addr', Int(0, 6 + self.distance_table_bits, 10)),
            ('wr_data', Int(0, (self.ij_bits + 1) * 2, 10)),
        ]

        distance_table_m = self.hw_components.create_memory_1r_1w()
        m.Instance(distance_table_m, distance_table_m.name, par, con)

        return m

    def create_stage4_yoto(self, cell_content_f: str, simulate: bool) -> Module:
        name = 'stage4_yoto'
        m = Module(name)

        clk = m.Input('clk')
        rst = m.Input('rst')

        th_idx = m.OutputReg('th_idx', self.th_bits)
        th_valid = m.OutputReg('th_valid')
        place = m.OutputReg('place')
        ib = m.OutputReg('ib', self.ij_bits)
        jb = m.OutputReg('jb', self.ij_bits)
        dist_counter = m.OutputReg('dist_counter', 6)
        b = m.OutputReg('b', self.node_bits)

        st3_th_idx = m.Input('st3_th_idx', self.th_bits)
        st3_th_valid = m.Input('st3_th_valid')
        st3_ib = m.Input('st3_ib', self.ij_bits + 1)
        st3_jb = m.Input('st3_jb', self.ij_bits + 1)
        st3_dist_counter = m.Input('st3_dist_counter', 6)
        st3_b = m.Input('st3_b', self.node_bits)

        st4_th_idx = m.Wire('st4_th_idx', self.th_bits)
        st4_place = m.Wire('st4_place')
        st4_ib = m.Wire('st4_ib', self.ij_bits)
        st4_jb = m.Wire('st4_jb', self.ij_bits)

        m.EmbeddedCode('')
        content = m.Wire('content')
        place_t = m.Wire('place_t')
        out_of_border_t = m.Wire('out_of_border_t')

        m.EmbeddedCode('')
        out_of_border_t.assign(OrList(
            st3_ib[self.ij_bits],
            st3_jb[self.ij_bits],
            st3_ib[0:self.ij_bits] > Int(self.n_lines - 1, self.ij_bits, 10),
            st3_jb[0:self.ij_bits] > Int(self.n_lines - 1, self.ij_bits, 10)
        ))
        place_t.assign(Uand(Cat(~content, out_of_border_t)))
        # i > n_cells_sqrt - 1 or j > n_cells_sqrt - 1 or i < 0 or j < 0

        m.EmbeddedCode('')
        st4_th_idx.assign(th_idx)
        st4_place.assign(place)
        st4_ib.assign(ib)
        st4_jb.assign(jb)

        m.Always(Posedge(clk))(
            If(rst)(
                th_idx(Int(0, th_idx.width, 10)),
                th_valid(Int(0, 1, 10)),
                place(Int(0, 1, 10)),
                ib(Int(0, ib.width, 10)),
                jb(Int(0, jb.width, 10)),
                dist_counter(Int(0, dist_counter.width, 10)),
                b(Int(0, b.width, 10)),
            ).Else(
                th_idx(st3_th_idx),
                th_valid(st3_th_valid),
                place(place_t),
                ib(st3_ib[0:self.ij_bits]),
                jb(st3_jb[0:self.ij_bits]),
                dist_counter(st3_dist_counter),
                b(st3_b),
            )
        )

        par = [
            ('width', 1),
            ('depth', self.th_bits + self.ij_bits * 2),
        ]
        if simulate:
            par.append(('read_f', 1))
            par.append(('init_file', cell_content_f))
        con = [
            ('rd_addr', Cat(st3_th_idx, st3_ib[0:self.ij_bits], st3_jb[0:self.ij_bits])),
            ('out', content),
            ('wr', st4_place),
            ('wr_addr', Cat(st4_th_idx, st4_ib, st4_jb)),
            ('wr_data', Int(1, 1, 10)),
        ]

        cells_m = self.hw_components.create_memory_1r_1w()
        m.Instance(cells_m, cells_m.name, par, con)

        return m