import { Prop, Component } from 'vue-property-decorator';
import Vue from 'vue';

/**
 * Shared Class for *List components
 *
 * @export
 * @class Partial
 * @extends {Vue}
 */

@Component
export default class Partial extends Vue {
  @Prop() public title!: string;
  @Prop() public showEmpty!: boolean;

  get hasTitle() {
    return !!this.title && this.title !== '';
  }

  get displayEmpty() {
    return !!this.showEmpty && this.showEmpty === true;
  }
}
